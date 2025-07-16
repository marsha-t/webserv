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

// Sets up initial fd_set for each serverFD
// Calls selectLoop 
void	ServerManager::start(void)
{
	fd_set master_fds;
	FD_ZERO(&master_fds);
	int max_fd = 0;

	for (size_t i = 0; i < _servers.size(); ++i)
	{
		int fd = _servers[i].getServerFD();
		FD_SET(fd, &master_fds);
		if (fd > max_fd)
			max_fd = fd;
	}
	debugMsg("Starting server. Initial max_fd = ", max_fd);
	selectLoop(master_fds, max_fd);
}

// Iterates over all FDs: 
// - if it's a listening socket, accept a new client
// - if it's a client socket, read and process request
void ServerManager::selectLoop(fd_set master_fds, int max_fd)
{
	fd_set read_fds;

	while (true)
	{
		read_fds = master_fds;
		debugMsg("Waiting for activity on fds (max_fd = ", max_fd);
		int ready = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
		std::ostringstream oss;
		oss << "select() returned " << ready << " ready fds";
		debugMsg(oss.str());
		if (ready < 0)
			fatalError("Select failed");
		
		for (int i = 0; i <= max_fd; ++i)
		{
			if (FD_ISSET(i, &read_fds))
			{
				if (isListeningSocket(i))
					acceptNewClient(i, master_fds, max_fd);
				else
				{
					if (!handleClientRead(i))
					{
						cleanupClient(i, master_fds, max_fd);
						debugMsg("Cleaned up FD = ", i);
						continue;
					}
					std::map<int, Request>::iterator it = _clientRequests.find(i);
					if (it != _clientRequests.end())
					{
						processClientRequest(i, it->second, master_fds, max_fd);
						cleanupClient(i, master_fds, max_fd);
						debugMsg("Cleaned up FD = ", i);
					}
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
void ServerManager::acceptNewClient(int serverFD, fd_set &master_fds, int &max_fd)
{
	int clientFD = accept(serverFD, NULL, NULL);
	if (clientFD < 0)
		fatalError("Accept failed");
	if (fcntl(clientFD, F_SETFL, O_NONBLOCK) < 0)
	{
		close(clientFD);
		fatalError("fctnl failed");
	}
	FD_SET(clientFD, &master_fds);
	if (clientFD > max_fd)
		max_fd = clientFD;

	const Server *server = getListeningServerByFD(serverFD);
	if (server)
		_clientToServer[clientFD] = server;
	debugMsg("Accepted new client: FD = ", clientFD);
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
			errorMsg("read failed");
			return false;
		}
		else if (bytesRead == 0)
			return false;
		buffer[bytesRead] = '\0';
		_clientBuffers[clientFD] += buffer;
	}

	// If headers are received, parse into Request
	std::string::size_type headerEnd = _clientBuffers[clientFD].find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		return true; // Headers not complete yet

	// Check if body is complete based on Content-Length or Transfer-Encoding
	std::string headers = _clientBuffers[clientFD].substr(0, headerEnd);
	Request tempReq; // Use a temporary request to parse headers only
	tempReq.parse(headers + "\r\n\r\n"); // Add CRLF to make it a valid request for parsing

	std::string contentLengthStr = tempReq.getHeader("content-length");
	std::string transferEncoding = tempReq.getHeader("transfer-encoding");

	if (!transferEncoding.empty() && transferEncoding == "chunked")
	{
		// For chunked, we need to read until the 0-length chunk is found
		if (_clientBuffers[clientFD].find("0\r\n\r\n", headerEnd) == std::string::npos)
			return true; // Not all chunks received yet
	}
	else if (!contentLengthStr.empty())
	{
		char *endptr;
		long contentLength = std::strtol(contentLengthStr.c_str(), &endptr, 10);
		if (*endptr != '\0' || contentLength < 0)
    		return false;
		if (_clientBuffers[clientFD].length() - (headerEnd + 4) < static_cast<size_t>(contentLength))
			return true; // Not all body received yet
	}

	// Full request received, parse and store
	Request req;
	if (!req.parse(_clientBuffers[clientFD]))
		return false; // invalid request → close connection
	_clientRequests[clientFD] = req;
	debugMsg("Full request parsed for FD = ", clientFD);
	return true;
}

// Matches correct ServerConfig and Route
// Obtain max body size if specified 
// Validate request body (using max body size + other checks)
// Dispatches request to appropriate handler
void ServerManager::processClientRequest(int clientFD, Request& request, fd_set &master_fds, int &max_fd) {
	std::map<int, const Server*>::iterator it = _clientToServer.find(clientFD);
	if (it == _clientToServer.end()) {
		errorMsg("No server associated with client", clientFD);
		return;
	}
	const Server* server = it->second;
	if (!server) {
		errorMsg("No server associated with client", clientFD);
		return;
	}

	const ServerConfig& selectedConfig = server->selectServer(request.getHeader("Host"));

	Route matchedRoute;
	if (!selectedConfig.matchRoute(request.getTarget(), matchedRoute)) {
		Response res;
		res.setError(404, selectedConfig);
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
		res.setError(request.getParseErrorCode(), selectedConfig);
		std::string responseStr = res.toString();
		write(clientFD, responseStr.c_str(), responseStr.size());
		return;
	}

	// Select handler and generate response
	Response response;
	RequestDispatcher dispatcher;
	IRequestHandler* handler = dispatcher.selectHandler(request, matchedRoute, selectedConfig);

	if (!handler) {
		errorMsg("No handler implemented", clientFD);
		response.setError(501, selectedConfig);
		std::string responseStr = response.toString();
		write(clientFD, responseStr.c_str(), responseStr.size());
		return;
	}

	handler->handle(request, response);
	delete handler;

	std::string responseStr = response.toString();
	ssize_t bytesSent = write(clientFD, responseStr.c_str(), responseStr.size());
	if (bytesSent < 0)
	{
		errorMsg("Write to client failed", clientFD);
        cleanupClient(clientFD, master_fds, max_fd);
		return ;
	}

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
void ServerManager::cleanupClient(int fd, fd_set &master_fds, int &max_fd)
{
	close(fd);
	FD_CLR(fd, &master_fds);
	if (fd == max_fd)
	{
		max_fd = 0;
		for (int i = FD_SETSIZE - 1; i >= 0; --i)
		{
			if (FD_ISSET(i, &master_fds))
			{
				max_fd = i;
				break;
			}
		}
	}

	_clientToServer.erase(fd);
	_clientBuffers.erase(fd);
	_clientRequests.erase(fd);
	debugMsg("Closed client: FD = ", fd);

}