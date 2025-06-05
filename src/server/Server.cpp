#include "../../includes/Server.hpp"

Server::Server(void): _serverFD(-1) {}
Server::Server(const Server &obj): _serverFD(obj._serverFD), _config(obj._config) {}
Server::Server(const ServerConfig &config): _serverFD(-1), _config(config) {}
Server &Server::operator=(const Server &obj)
{
	if (this != &obj)
	{
		_serverFD = obj._serverFD;
		_config = obj._config;
	}
	return (*this);
}

Server::~Server(void) 
{
	if (_serverFD != -1)
		close(_serverFD);
}

int Server::getServerFD(void) const { return _serverFD; }
const ServerConfig &Server::getConfig(void) const {return _config; }

void Server::initSocket(void)
{
	_serverFD = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverFD == -1)
		throw std::runtime_error("Failed to create listening socket");
	int opt = 1;
	if (setsockopt(_serverFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) 
		throw std::runtime_error("Failed to set socket options");
	sockaddr_in address;
	std::memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(_config.getPort());
	if (inet_pton(AF_INET, _config.getHost().c_str(), &address.sin_addr) <= 0)
		throw std::runtime_error("Invalid IP address: " + _config.getHost());
	if (bind(_serverFD, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0)
		throw std::runtime_error("Failed to bind socket");
	if (listen(_serverFD, SOMAXCONN) < 0)
		throw std::runtime_error("Failed to listen on socket");
	std::cout << "Listening on " << _config.getPort() << std::endl; // TODO: remove later
}