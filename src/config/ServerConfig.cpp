#include "ServerConfig.hpp"

ServerConfig::ServerConfig(void) {}
ServerConfig::ServerConfig(const ServerConfig &obj): _host(obj._host), _port(obj._port), _serverName(obj._serverName), _routes(obj._routes), _errorPages(obj._errorPages) {}
ServerConfig::~ServerConfig(void) {}
ServerConfig &ServerConfig::operator=(const ServerConfig &obj)
{
	if (this != &obj)
	{
		_host = obj._host;
		_port = obj._port;
		_serverName = obj._serverName;
		_routes = obj._routes;
		_errorPages = obj._errorPages;
	}
	return (*this);
}

void	ServerConfig::setHost(const std::string &host) { _host = host; }
void	ServerConfig::setPort(int port) { _port = port; }
void	ServerConfig::setServerName(const std::string &serverName) { _serverName = serverName; }
void	ServerConfig::addErrorPage(int code, const std::string &filepath) { _errorPages[code] = filepath; }
void	ServerConfig::addRoute(const Route &route) { _routes.push_back(route); }
void	ServerConfig::addClientMaxBodySize(unsigned int clientMaxBodySize) { _clientMaxBodySize = clientMaxBodySize; }

	const std::string &ServerConfig::getHost(void) const { return _host; }
int	ServerConfig::getPort(void) const { return _port; }
const std::string &ServerConfig::getServerName(void) const { return _serverName; }
const std::vector<Route> &ServerConfig::getRoutes(void) const { return _routes; }
const std::map<int, std::string> &ServerConfig::getErrorPages(void) const { return _errorPages; }
unsigned int ServerConfig::getClientMaxBodySize(void) const { return _clientMaxBodySize; }