#include "../../includes/ServerConfig.hpp"

ServerConfig::ServerConfig(void): _hasClientMaxBodySize(false) {}
ServerConfig::ServerConfig(const ServerConfig &obj): _host(obj._host), _port(obj._port), _serverNames(obj._serverNames), _errorPages(obj._errorPages), _routes(obj._routes), _clientMaxBodySize(obj._clientMaxBodySize), _hasClientMaxBodySize(obj._hasClientMaxBodySize) {}
ServerConfig::~ServerConfig(void) {}
ServerConfig &ServerConfig::operator=(const ServerConfig &obj)
{
	if (this != &obj)
	{
		_host = obj._host;
		_port = obj._port;
		_serverNames = obj._serverNames;
		_errorPages = obj._errorPages;
		_routes = obj._routes;
		_clientMaxBodySize = obj._clientMaxBodySize;
		_hasClientMaxBodySize = obj._hasClientMaxBodySize;
	}
	return (*this);
}

void	ServerConfig::setHost(const std::string &host) { _host = host; }
void	ServerConfig::setPort(int port) { _port = port; }
void	ServerConfig::addServerName(const std::string &serverName) { _serverNames.push_back(serverName); }
void	ServerConfig::addErrorPage(int code, const std::string &filepath) { _errorPages[code] = filepath; }
void	ServerConfig::addRoute(const Route &route) { _routes.push_back(route); }
void	ServerConfig::setClientMaxBodySize(std::size_t clientMaxBodySize) 
{ 
	_clientMaxBodySize = clientMaxBodySize; 
	_hasClientMaxBodySize = true;
}

const std::string &ServerConfig::getHost(void) const { return _host; }
int	ServerConfig::getPort(void) const { return _port; }
const std::vector<std::string> &ServerConfig::getServerNames(void) const { return _serverNames; }
const std::vector<Route> &ServerConfig::getRoutes(void) const { return _routes; }
const std::map<int, std::string> &ServerConfig::getErrorPages(void) const { return _errorPages; }
std::size_t ServerConfig::getClientMaxBodySize(void) const { return _clientMaxBodySize; }
bool ServerConfig::hasClientMaxBodySize(void) const { return _hasClientMaxBodySize; }

bool ServerConfig::matchRoute(const std::string &target, Route &matchedRoute) const
{
	size_t longestMatchLen = 0;
	const Route *bestMatch = NULL;

	for (size_t i = 0; i < _routes.size(); ++i)
	{
		const std::string &location = _routes[i].getLocation();
		if (target.compare(0, location.size(), location) == 0)
		{
			if (location == "/" || target.size() == location.size() ||  target[location.size()] == '/')
			{
				if (location.size() > longestMatchLen)
				{
					bestMatch = &_routes[i];
					longestMatchLen = location.size();
				}
			}
		}
	}
	if (bestMatch)
	{
		matchedRoute = *bestMatch;
		return true;
	}
	return false;
}

