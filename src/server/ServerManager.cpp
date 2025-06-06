#include "../../includes/ServerManager.hpp"

ServerManager::ServerManager(void) {}
ServerManager::ServerManager(const ServerManager &obj): _servers(obj._servers), _serverFDs(obj._serverFDs) {}
ServerManager::ServerManager(const std::vector<ServerConfig> &configs)
{
	for (size_t i = 0; i < configs.size(); ++i)
		_servers.push_back(Server(configs[i]));
}

ServerManager::~ServerManager(void) 
{
	for (size_t i = 0; i < _serverFDs.size(); ++i)
		close(_serverFDs[i]);
}

ServerManager &ServerManager::operator=(const ServerManager &obj)
{
	if (this != &obj)
	{
		_servers = obj._servers;
		_serverFDs = obj._serverFDs;
	}
	return (*this);
}

void	ServerManager::setup(void)
{
	try
	{
		for (size_t i = 0; i < _servers.size(); ++i)
		{
			_servers[i].initSocket();
			_serverFDs.push_back(_servers[i].getServerFD());
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error setting up server[" << i << "]: " << e.what() << std::endl;
	}
}

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

	while (true)
	{
		int ready = poll(&fds[0], fds.size(), -1);
		if (ready < 0)
			throw std::runtime_error("Poll failed");
		for (size_t i = 0; i < fds.size(); ++i)
		{
			int fd = fds[i].fd;
			if (fds[i].revents & POLLIN)
			{
				if (isListeningSocket(fd))
					acceptNewClient(fd, fds);
				else
				{
					Request req;
					if (!handleClientRead(fd, req))
					{
						close(fd);
						fds.erase(fds.begin() + i);
						_clientToServer.erase(fd)
						--i;
						continue;
					}
					processClientRequest(fd, req);
					close(fd);
					fds.erase(fds.begin() + i);
					_clientToServer.erase(fd)
					--i;
				}
			}
		}
	}
}

bool ServerManager::isListeningSocket(int fd) const
{
	for (size_t i = 0; i < _serverFDs.size(); ++i)
	{
		if (_serverFDs[i] == fd)
			return true;
	}
	return false;
}

void ServerManager::acceptNewClient(int clientFD, std::vector<pollfd>& fds)
{
	int clientFD = accept(serverFD, NULL, NULL);
	if (clientFD < 0)
		throw std::runtime_error("Accept failed"); // consider changing to perror
	if (fcntl(clientFD, F_SETFL, O_NONBLOCK) < 0)
	{
		close(clientFD);
		throw std::runtime_error("fcntl failed"); // consider changing to perror
	}
	struct pollfd clientPollFD;
	clientPollFD.fd = clientFD;
	clientPollFD.events = POLLIN;
	clientPollFD.revents = 0;
	fds.push_back(clientPollFD);

	const Server *server = getServerByFD(serverFD);
	if (server)
		_clientToServer[clientFD] = server;
}

bool ServerManager::handleClientRead(int clientFD, Request& requestOut)
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
			perror("read error"); // throw std::runtime_error ?
			return false;
		}
		else if (bytesRead == 0) // client disconnected
			return false;

		buffer[bytesRead] = '\0';
		_clientBuffers[clientFD] += buffer;
		if (_clientBuffers[clientFD].find("\r\n\r\n") != std::string::npos)
			break;
	}

	// If we still haven't received the full request, return and wait for more
	if (_clientBuffers[clientFD].find("\r\n\r\n") == std::string::npos)
		return true; // keep polling for more data

	if (!requestOut.parse(_clientBuffers[clientFD]))
	{
		std::cerr << "Failed to parse request from fd: " << clientFD << std::endl;
		return false;
	}
	_clientRequests[clientFD] = requestOut;
	return true;
}

void ServerManager::processClientRequest(int clientFD, const Request& request)
{
	std::map<int, const Server*>::iterator it = _clientToServer.find(clientFD);
	if (it == _clientToServer.end())
	{
		std::cerr << "No server config found for client fd: " << clientFD << std::endl;
		return;
	}
	const ServerConfig& config = it->second->getConfig();

	// Match route (simplified for now)
	Route matchedRoute;
	if (!config.matchRoute(request.getTarget(), matchedRoute)) // TODO to build matchRoute
	{
		std::cerr << "No matching route for URI: " << request.getTarget() << std::endl;
		// TODO: Build 404 response here
		return;
	}

	Response response;
	RequestDispatcher dispatcher;
	IRequestHandler* handler = dispatcher.selectHandler(request, matchedRoute);
	if (!handler)
	{
		std::cerr << "No valid handler found for request" << std::endl;
		// TODO: Build 501 Not Implemented
		return;
	}

	handler->handle(request, response);
	delete handler;

	std::string responseStr = response.toString();
	ssize_t bytesSent = write(clientFD, responseStr.c_str(), responseStr.size());
	if (bytesSent < 0)
		perror("write failed");
}

