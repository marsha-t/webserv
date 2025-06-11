#ifndef SERVER_HPP
#define SERVER_HPP

#include "ServerConfig.hpp"

class Server 
{
	public:
		// Constructor
		Server(void);
		Server(const Server &obj);
		Server(const ServerConfig &configs);
		Server(const std::vector<ServerConfig> &configs);

		// Operator
		Server &operator=(const Server &obj);

		// Destructor
		~Server(void);
		
		// Getters
		int getServerFD(void) const;
		const std::vector<ServerConfig> &getConfigs(void) const;
		
		// Others
		void initSocket(void);
		const ServerConfig& selectServer(const std::string &hostHeader) const;

	private: 
		int _serverFD;
		std::vector<ServerConfig> _configs;
		std::map<std::string, ServerConfig> _nameToConfig;

};

#endif