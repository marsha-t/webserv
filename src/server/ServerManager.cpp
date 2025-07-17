#include "../../includes/ServerManager.hpp"

ServerManager::ServerManager(void)
{
	FD_ZERO(&_master_read_fds);
	FD_ZERO(&_master_write_fds);
	_max_fd = 0;
}
ServerManager::ServerManager(const ServerManager &obj)
	: _servers(obj._servers),
	  _clientToServer(obj._clientToServer),
	  _clientBuffers(obj._clientBuffers),
	  _clientRequests(obj._clientRequests),
	  _clientResponses(obj._clientResponses),
	  _responseOffsets(obj._responseOffsets),
	  _max_fd(obj._max_fd)
{
	_master_read_fds = obj._master_read_fds;
	_master_write_fds = obj._master_write_fds;
}

ServerManager& ServerManager::operator=(const ServerManager &obj)
{
	if (this != &obj)
	{
		_servers = obj._servers;
		_clientToServer = obj._clientToServer;
		_clientBuffers = obj._clientBuffers;
		_clientRequests = obj._clientRequests;
		_clientResponses = obj._clientResponses;
		_responseOffsets = obj._responseOffsets;
		_max_fd = obj._max_fd;
		_master_read_fds = obj._master_read_fds;
		_master_write_fds = obj._master_write_fds;
	}
	return *this;
}

ServerManager::~ServerManager(void)
{
	for (size_t i = 0; i < _servers.size(); ++i)
		close(_servers[i].getServerFD());
}

ServerManager::ServerManager(const std::vector<ServerConfig> &configs): _max_fd(0)
{
	FD_ZERO(&_master_read_fds);
	FD_ZERO(&_master_write_fds);

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
void ServerManager::start(void)
{
	FD_ZERO(&_master_read_fds);
	FD_ZERO(&_master_write_fds);
	_max_fd = 0;

	for (size_t i = 0; i < _servers.size(); ++i)
	{
		int fd = _servers[i].getServerFD();
		FD_SET(fd, &_master_read_fds);
		trackMaxFd(fd);
	}
	debugMsg("Starting server. Initial max_fd = ", _max_fd);
	selectLoop();
}

// Iterates over all FDs: 
// - if it's a listening socket, accept a new client
// - if it's a client socket, read and process request
void ServerManager::selectLoop(void)
{
	while (true)
	{
		fd_set read_fds = _master_read_fds;
		fd_set write_fds = _master_write_fds;

		debugMsg("Waiting for activity on fds (max_fd = ", _max_fd);
		int ready = select(_max_fd + 1, &read_fds, &write_fds, NULL, NULL);
		if (ready < 0)
			fatalError("Select failed");

		for (int fd = 0; fd <= _max_fd; ++fd)
		{
			if (FD_ISSET(fd, &read_fds))
			{
				if (isListeningSocket(fd))
					acceptNewClient(fd);
				else
				{
					if (!handleClientRead(fd))
					{
						cleanupClient(fd);
						debugMsg("Cleaned up FD = ", fd);
						continue;
					}
					std::map<int, Request>::iterator it = _clientRequests.find(fd);
					if (it != _clientRequests.end())
						processClientRequest(fd, it->second);
				}
			}
			else if (FD_ISSET(fd, &write_fds))
				handleClientWrite(fd);
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
void ServerManager::acceptNewClient(int serverFD)
{
	int clientFD = accept(serverFD, NULL, NULL);
	if (clientFD < 0)
		fatalError("Accept failed");

	if (fcntl(clientFD, F_SETFL, O_NONBLOCK) < 0)
	{
		close(clientFD);
		fatalError("fcntl failed");
	}

	FD_SET(clientFD, &_master_read_fds);
	trackMaxFd(clientFD);

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
	std::string& buffer = _clientBuffers[clientFD];

	if (!readFromClient(clientFD, buffer))
		return false;

	if (!headersComplete(buffer))
		return true;

	Request tempReq;
	if (!parseTempHeaders(buffer, tempReq))
		return false;

	if (!isBodyComplete(buffer, tempReq))
		return true;

	if (!parseFullRequest(buffer, _clientRequests[clientFD]))
		return false;

	return true;
}

bool ServerManager::readFromClient(int fd, std::string& buffer)
{
	char tempBuf[1024];
	std::memset(tempBuf, 0, sizeof(tempBuf));

	ssize_t bytesRead = read(fd, tempBuf, sizeof(tempBuf) - 1);
	if (bytesRead < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return true;
		errorMsg("Read failed on FD = ", fd);
		return false;
	}
	else if (bytesRead == 0)
	{
		debugMsg("Client closed connection: FD = ", fd);
		return false;
	}

	buffer.append(tempBuf, bytesRead);
	return true;
}

bool ServerManager::headersComplete(const std::string& buffer)
{
	return buffer.find("\r\n\r\n") != std::string::npos;
}

bool ServerManager::parseTempHeaders(const std::string& buffer, Request& tempReq)
{
	std::string::size_type headerEnd = buffer.find("\r\n\r\n");
	std::string headers = buffer.substr(0, headerEnd + 4);

	if (!tempReq.parse(headers))
	{
		errorMsg("Invalid headers");
		return false;
	}
	return true;
}

bool ServerManager::isBodyComplete(const std::string& buffer, const Request& tempReq)
{
	std::string::size_type headerEnd = buffer.find("\r\n\r\n");
	std::string contentLengthStr = tempReq.getHeader("content-length");
	std::string transferEncoding = tempReq.getHeader("transfer-encoding");

	if (transferEncoding == "chunked")
		return buffer.find("0\r\n\r\n", headerEnd) != std::string::npos;

	if (!contentLengthStr.empty())
	{
		char* endptr;
		long contentLength = std::strtol(contentLengthStr.c_str(), &endptr, 10);
		if (*endptr != '\0' || contentLength < 0)
		{
			errorMsg("Invalid Content-Length");
			return false;
		}
		std::size_t bodySize = buffer.length() - (headerEnd + 4);
		return bodySize >= static_cast<size_t>(contentLength);
	}
	// No body expected
	return true;
}

bool ServerManager::parseFullRequest(const std::string& buffer, Request& requestOut)
{
	if (!requestOut.parse(buffer))
	{
		errorMsg("Full request parse failed");
		return false;
	}
	return true;
}

// Matches correct ServerConfig and Route
// Obtain max body size if specified 
// Validate request body (using max body size + other checks)
// Dispatches request to appropriate handler
void ServerManager::processClientRequest(int clientFD, Request& request)
{
	const ServerConfig* config = getSelectedConfig(clientFD, request);
	if (!config)
		return;

	Route matchedRoute;
	if (!matchRouteOrRespond404(clientFD, request, *config, matchedRoute))
		return;

	std::size_t maxBodySize = getMaxBodySize(matchedRoute, *config);
	if (!validateRequestOrRespondError(clientFD, request, *config, maxBodySize))
		return;

	generateResponseAndBuffer(clientFD, request, matchedRoute, *config);
}

const ServerConfig* ServerManager::getSelectedConfig(int clientFD, const Request& request)
{
	std::map<int, const Server*>::iterator it = _clientToServer.find(clientFD);
	if (it == _clientToServer.end() || !it->second)
	{
		errorMsg("No server associated with client", clientFD);
		return NULL;
	}

	const Server* server = it->second;
	return &server->selectServer(request.getHeader("Host"));
}

bool ServerManager::matchRouteOrRespond404(int clientFD, const Request& request, const ServerConfig& config, Route& matchedRoute)
{
	if (!config.matchRoute(request.getTarget(), matchedRoute))
	{
		Response res;
		res.setError(404, config);
		bufferResponse(clientFD, res.toString());
		return false;
	}
	return true;
}

std::size_t ServerManager::getMaxBodySize(const Route& route, const ServerConfig& config)
{
	if (route.hasClientMaxBodySize())
		return route.getClientMaxBodySize();
	else if (config.hasClientMaxBodySize())
		return config.getClientMaxBodySize();
	else
		return DEFAULT_MAX_BODY_SIZE;
}

bool ServerManager::validateRequestOrRespondError(int clientFD, Request& request, const ServerConfig& config, std::size_t maxBodySize)
{
	if (!request.validateBody(maxBodySize))
	{
		Response res;
		res.setError(request.getParseErrorCode(), config);
		bufferResponse(clientFD, res.toString());
		return false;
	}
	return true;
}

void ServerManager::generateResponseAndBuffer(int clientFD, const Request& request, const Route& route, const ServerConfig& config)
{
	Response response;
	RequestDispatcher dispatcher;
	IRequestHandler* handler = dispatcher.selectHandler(request, route, config);

	if (!handler)
	{
		errorMsg("No handler implemented", clientFD);
		response.setError(501, config);
		bufferResponse(clientFD, response.toString());
		return;
	}

	handler->handle(request, response);
	delete handler;

	bufferResponse(clientFD, response.toString());
}

void ServerManager::bufferResponse(int fd, const std::string& responseStr)
{
	_clientResponses[fd] = responseStr;
	_responseOffsets[fd] = 0;
	FD_SET(fd, &_master_write_fds);
	trackMaxFd(fd);
}

void ServerManager::handleClientWrite(int fd)
{
	std::map<int, std::string>::iterator respIt = _clientResponses.find(fd);
	if (respIt == _clientResponses.end())
	{
		errorMsg("No response buffer found for FD = ", fd);
		cleanupClient(fd);  // To prevent stuck connection
		return;
	}

	std::string &response = respIt->second;
	size_t &offset = _responseOffsets[fd];

	ssize_t bytesSent = write(fd, response.c_str() + offset, response.size() - offset);

	if (bytesSent < 0)
	{
		errorMsg("Write failed on FD = ", fd);
		cleanupClient(fd);
		return;
	}

	offset += bytesSent;

	// If not done writing yet, wait for next select()
	if (offset < response.size())
	{
		debugMsg("Partial write: waiting to finish FD = ", fd);
		return;
	}

	// Full response sent — cleanup
	debugMsg("Full response sent to FD = ", fd);
	cleanupClient(fd);
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
void ServerManager::cleanupClient(int fd)
{
	close(fd);
	FD_CLR(fd, &_master_read_fds);
	FD_CLR(fd, &_master_write_fds);

	_clientToServer.erase(fd);
	_clientBuffers.erase(fd);
	_clientRequests.erase(fd);
	_clientResponses.erase(fd);
	_responseOffsets.erase(fd);

	debugMsg("Closed client: FD = ", fd);

	// Recalculate _max_fd across both read and write sets
	if (fd == _max_fd)
	{
		_max_fd = 0;
		for (int i = FD_SETSIZE - 1; i >= 0; --i)
		{
			if (FD_ISSET(i, &_master_read_fds) || FD_ISSET(i, &_master_write_fds))
			{
				_max_fd = i;
				break;
			}
		}
	}
}


void ServerManager::trackMaxFd(int fd)
{
	if (fd > _max_fd)
		_max_fd = fd;
}


