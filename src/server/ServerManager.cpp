#include "../../includes/ServerManager.hpp"

ServerManager::ServerManager(void) {}
ServerManager::ServerManager(const ServerManager &obj): _servers(obj._servers), _clientToServer(obj._clientToServer), _clientBuffers(obj._clientBuffers), _clientRequests(obj._clientRequests) {}
ServerManager::ServerManager(const std::vector<ServerConfig> &configs)
{
	
	std::map<std::pair<std::string, int>, std::vector<ServerConfig> > grouped;
	for (size_t i = 0; i < configs.size(); ++i)
	{
		std::pair<std::string, int> key = std::make_pair(configs[i].getHost(), configs[i].getPort());
		grouped[key].push_back(configs[i]);
	}
	for (std::map<std::pair<std::string, int>, std::vector<ServerConfig> >::iterator it = grouped.begin(); it != grouped.end(); ++it)
	{
		Server server(it->second);
		_servers.push_back(server);
	}
}

ServerManager::~ServerManager(void) 
{
	for (size_t i = 0; i < _servers.size(); ++i)
		close(_servers[i].getServerFD());
}

ServerManager &ServerManager::operator=(const ServerManager &obj)
{
	if (this != &obj)
	{
		_servers = obj._servers;
		_clientToServer = obj._clientToServer;
		_clientBuffers = obj._clientBuffers;
		_clientRequests = obj._clientRequests;
	}
	return (*this);
}

/// Initialises sockets on each Server and called once at startup in main()
void	ServerManager::setup(void)
{
	size_t i = 0;
	try
	{
		for (; i < _servers.size(); ++i)
		{
			_servers[i].initSocket();
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error setting up server[" << i << "] — ";
		if (i < _servers.size())
		{
			const std::vector<ServerConfig> &configs = _servers[i].getConfigs();
			if (!configs.empty())
			{
				std::cerr << "host: " << configs[0].getHost()
						  << ", port: " << configs[0].getPort() << " — ";
			}
		}
		std::cerr << e.what() << std::endl;
	}
}

// Sets up initial pollfd array for each serverFD
// Calls pollLoop 
void	ServerManager::start(void)
{
	std::vector<pollfd> fds;
	for (size_t i = 0; i < _servers.size(); ++i)
	{
		pollfd serverPollFD;
		serverPollFD.fd = _servers[i].getServerFD();
		serverPollFD.events = POLLIN;
		serverPollFD.revents = 0;
		fds.push_back(serverPollFD);
	}	
	pollLoop(fds);
}

// Iterates over all FDs: 
// - if it's a listening socket, accept a new client
// - if it's a client socket, read and process request
void ServerManager::pollLoop(std::vector<struct pollfd> &fds)
{
	while (true)
	{
		int ready = poll(&fds[0], fds.size(), -1);
		if (ready < 0)
			throw std::runtime_error("Poll failed");
		
		for (size_t i = 0; i < fds.size(); ++i)
		{
			int fd = fds[i].fd;
			if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) // errors
			{
				cleanupClient(fd, fds, i);
				continue;
			}
			else if (fds[i].revents & POLLIN) // to check after errors; in case both coexist
			{
				try
				{
					if (isListeningSocket(fd))
						acceptNewClient(fd, fds);
					else
					{
						if (!handleClientRead(fd))
						{
							cleanupClient(fd, fds, i);
							continue;
						}
						std::map<int, Request>::iterator it = _clientRequests.find(fd);
						if (it != _clientRequests.end())
						{
							processClientRequest(fd, it->second);
							cleanupClient(fd, fds, i);
							
						}
					}
				}
				catch (const std::exception &e)
				{
					std::cerr << "Error handling fd " << fd << ": " << e.what() << std::endl;
					cleanupClient(fd, fds, i);
				}
			}
		}
	}
}

bool ServerManager::isListeningSocket(int fd) const
{
	for (size_t i = 0; i < _servers.size(); ++i)
	{
		if (_servers[i].getServerFD() == fd)
			return true;
	}
	return false;
}

// Accepts client connection and adds clientFD to fds for monitoring
// Stores mapping of client to server in _clientToServer
void ServerManager::acceptNewClient(int serverFD, std::vector<pollfd>& fds)
{
	int clientFD = accept(serverFD, NULL, NULL);
	if (clientFD < 0)
		throw std::runtime_error("Accept failed"); // TODO consider changing to perror
	if (fcntl(clientFD, F_SETFL, O_NONBLOCK) < 0)
	{
		close(clientFD);
		throw std::runtime_error("fcntl failed"); // TODO consider changing to perror
	}
	struct pollfd clientPollFD;
	clientPollFD.fd = clientFD;
	clientPollFD.events = POLLIN;
	clientPollFD.revents = 0;
	fds.push_back(clientPollFD);

	const Server *server = getListeningServerByFD(serverFD);
	if (server)
		_clientToServer[clientFD] = server;
}

// Reads incoming data from client socket 
// Stores raw data into _clientBuffers
// If headers are complete, parses into Request and stores it in _clientRequests
// If headers are incomplete, return to poll loop to read some more 
bool ServerManager::handleClientRead(int clientFD)
{
	char buffer[1024];
	ssize_t bytesRead;

	while (true)
	{
		std::memset(buffer, 0, sizeof(buffer));
		bytesRead = read(clientFD, buffer, sizeof(buffer) - 1);
		if (bytesRead < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break; // No more data for now — return to poll()
			perror("read error"); // TODO throw std::runtime_error?
			return false;
		}
		else if (bytesRead == 0) // client disconnected
			return false;

		buffer[bytesRead] = '\0';
		_clientBuffers[clientFD] += buffer;
	}

	// If headers are received, parse into Request
	if (_clientBuffers[clientFD].find("\r\n\r\n") != std::string::npos)
	{
		Request req;
		if (!req.parse(_clientBuffers[clientFD]))
			return false; // invalid request → close connection
		_clientRequests[clientFD] = req;
	}
	// If headers not received, return true to continue reading
	return true;
}

// Matches correct ServerConfig and Route
// Obtain max body size if specified 
// Validate request body (using max body size + other checks)
// Dispatches request to appropriate handler
void ServerManager::processClientRequest(int clientFD, Request& request) {
	std::map<int, const Server*>::iterator it = _clientToServer.find(clientFD);
	if (it == _clientToServer.end()) {
		std::cerr << "No server for fd " << clientFD << std::endl;
		return;
	}
	const Server* server = it->second;
	if (!server) {
		std::cerr << "No server for fd " << clientFD << std::endl;
		return;
	}

	const ServerConfig& selectedConfig = server->selectServer(request.getHeader("Host"));

	Route matchedRoute;
	if (!selectedConfig.matchRoute(request.getTarget(), matchedRoute)) {
		Response res;
		res.setError(404, "Not Found");
		std::string responseStr = res.toString();
		write(clientFD, responseStr.c_str(), responseStr.size());
		return;
	}

	// Validate request body: first determine correct client_max_body_size, before running validateBody()
	std::size_t maxBodySize;
	if (matchedRoute.hasClientMaxBodySize())
		maxBodySize = matchedRoute.getClientMaxBodySize();
	else if (selectedConfig.hasClientMaxBodySize())
		maxBodySize = selectedConfig.getClientMaxBodySize();
	else
		maxBodySize = DEFAULT_MAX_BODY_SIZE;

	if (!request.validateBody(maxBodySize)) {
		Response res;
		res.setError(413, "Payload Too Large");
		std::string responseStr = res.toString();
		write(clientFD, responseStr.c_str(), responseStr.size());
		return;
	}

	// Select handler and generate response
	Response response;
	RequestDispatcher dispatcher;
	IRequestHandler* handler = dispatcher.selectHandler(request, matchedRoute);

	if (!handler) {
		response.setError(501, "Not Implemented");
		std::string responseStr = response.toString();
		write(clientFD, responseStr.c_str(), responseStr.size());
		return;
	}

	handler->handle(request, response);
	delete handler;

	std::string responseStr = response.toString();
	ssize_t bytesSent = write(clientFD, responseStr.c_str(), responseStr.size());
	if (bytesSent < 0)
		perror("write failed");

	_clientBuffers.erase(clientFD);
	_clientRequests.erase(clientFD);
}

// Used to get Server object for a given listening socket
// Does not work if given a clientFD
const Server* ServerManager::getListeningServerByFD(int fd) const
{
	for (size_t i = 0; i < _servers.size(); ++i)
	{
		if (_servers[i].getServerFD() == fd)
			return &_servers[i];
	}
	return NULL;
}

// Closes and removes client FD from ServerManager
void ServerManager::cleanupClient(int fd, std::vector<struct pollfd> &fds, size_t &i)
{
	close(fd);
	fds.erase(fds.begin() + i);
	_clientToServer.erase(fd);
	_clientBuffers.erase(fd);
	_clientRequests.erase(fd);
	--i;
}
