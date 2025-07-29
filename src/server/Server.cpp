#include "../../includes/Server.hpp"

Server::Server(void): _serverFD(-1) {}

Server::Server(const Server &obj) : _serverFD(obj._serverFD), _configs(obj._configs)
{
	for (size_t i = 0; i < _configs.size(); ++i)
	{
		const std::vector<std::string>& names = _configs[i].getServerNames();
		for (size_t j = 0; j < names.size(); ++j)
		{
			const std::string& name = names[j];
			if (name.empty())
				continue;
			if (_nameToConfig.count(name) != 0)
				throw std::runtime_error("Duplicate server_name '" + name + "' in copy constructor");
			_nameToConfig.insert(std::make_pair(name, &_configs[i]));
		}
	}
}

Server::Server(const std::vector<ServerConfig>& configs) : _serverFD(-1), _configs(configs)
{
	for (size_t i = 0; i < _configs.size(); ++i)
	{
		const std::vector<std::string>& names = _configs[i].getServerNames();
		for (size_t j = 0; j < names.size(); ++j)
		{
			const std::string& name = names[j];
			if (name.empty())
				continue;
			if (_nameToConfig.count(name) != 0)
				throw std::runtime_error("Duplicate server_name '" + name + "' for same host:port");
			_nameToConfig.insert(std::make_pair(name, &_configs[i]));
		}
	}
}

Server& Server::operator=(const Server& obj)
{
	if (this != &obj)
	{
		_serverFD = obj._serverFD;
		_configs = obj._configs;
		_nameToConfig.clear();

		for (size_t i = 0; i < _configs.size(); ++i)
		{
			const std::vector<std::string>& names = _configs[i].getServerNames();
			for (size_t j = 0; j < names.size(); ++j)
			{
				const std::string& name = names[j];
				if (name.empty())
					continue;
				if (_nameToConfig.count(name) != 0)
					throw std::runtime_error("Duplicate server_name '" + name + "' for same host:port");
				_nameToConfig.insert(std::make_pair(name, &_configs[i]));
			}
		}
	}
	return *this;
}

Server::~Server(void) 
{
	if (_serverFD != -1)
		close(_serverFD);
}

int Server::getServerFD(void) const { return _serverFD; }
const std::vector<ServerConfig> &Server::getConfigs(void) const {return _configs; }

void Server::initSocket(void)
{
	try
	{
		if (_configs.empty())
	    	throw std::runtime_error("No server configuration provided");
		_serverFD = socket(AF_INET, SOCK_STREAM, 0);
		if (_serverFD == -1)
			throw std::runtime_error("Failed to create listening socket");
		int opt = 1;
		if (setsockopt(_serverFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) 
			throw std::runtime_error("Failed to set socket options");
		sockaddr_in address;
		std::memset(&address, 0, sizeof(address));
		address.sin_family = AF_INET;
		address.sin_port = htons(_configs[0].getPort());
		if (inet_pton(AF_INET, _configs[0].getHost().c_str(), &address.sin_addr) <= 0)
			throw std::runtime_error("Invalid IP address: " + _configs[0].getHost());
		if (bind(_serverFD, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0)
			throw std::runtime_error("Failed to bind socket");
		if (listen(_serverFD, SOMAXCONN) < 0)
			throw std::runtime_error("Failed to listen on socket");
	}
	catch (...)
	{
		if (_serverFD != -1)
			close(_serverFD);
		throw;
	}
}

const ServerConfig& Server::selectServer(const std::string &hostHeader) const
{
	std::string hostname = hostHeader;
	std::string::size_type colon = hostname.find(':');
	if (colon != std::string::npos)
		hostname = hostname.substr(0, colon);

	// Try exact match from map
	std::map<std::string, const ServerConfig*>::const_iterator it = _nameToConfig.find(hostname);
	if (it != _nameToConfig.end()) {
		debugMsg("Matched server config: " + hostname);
		return *(it->second);
	}

	// Fallback - search configs for one that has "default_server" as a server_name
	for (size_t i = 0; i < _configs.size(); ++i)
	{
		const std::vector<std::string>& names = _configs[i].getServerNames();
		for (size_t j = 0; j < names.size(); ++j)
		{
			if (names[j] == "default_server")
			{
				debugMsg("Using default_server fallback");
				return _configs[i];
			}
		}
	}

	// Absolute fallback - return first config
	debugMsg("No matching or default server; using first config as fallback");
	return _configs[0];
}
