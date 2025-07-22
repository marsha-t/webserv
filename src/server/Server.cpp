#include "../../includes/Server.hpp"

Server::Server(void): _serverFD(-1) {}

Server::Server(const Server &obj): _serverFD(obj._serverFD), _configs(obj._configs)
{
	for (size_t i = 0; i < _configs.size(); ++i)
	{
		const std::vector<std::string> &names = _configs[i].getServerNames();
		for (size_t j = 0; j < names.size(); ++j)
		{
			if (_nameToConfig.count(names[j]) == 0)
				_nameToConfig.insert(std::make_pair(names[j], _configs[i]));
			else 
				std::cerr << "Warning: Duplicate server_name '" << names[j] << "' ignored\n";
		}
	}
}

Server::Server(const std::vector<ServerConfig> &configs): _serverFD(-1), _configs(configs)
{
	for (size_t i = 0; i < configs.size(); ++i)
	{
		const std::vector<std::string> &names = configs[i].getServerNames();
		for (size_t j = 0; j < names.size(); ++j)
		{
			if (_nameToConfig.count(names[j]) == 0)  // only insert first occurrence
				_nameToConfig.insert(std::make_pair(names[j], configs[i]));
		}
	}
}

Server &Server::operator=(const Server &obj)
{
	if (this != &obj)
	{
		_serverFD = obj._serverFD;
		_configs = obj._configs;
		_nameToConfig.clear();
		for (size_t i = 0; i < _configs.size(); ++i)
		{
			const std::vector<std::string> &names = _configs[i].getServerNames();
			for (size_t j = 0; j < names.size(); ++j)
			{
				if (_nameToConfig.count(names[j]) == 0)
					_nameToConfig.insert(std::make_pair(names[j], _configs[i]));
				else 
					std::cerr << "Warning: Duplicate server_name '" << names[j] << "' ignored\n";
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
	std::map<std::string, ServerConfig>::const_iterator it = _nameToConfig.find(hostname);
	if (it != _nameToConfig.end())
		return it->second;
	return _configs[0];
}
