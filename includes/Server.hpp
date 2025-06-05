#ifndef SERVER_HPP
#define SERVER_HPP

#include "ServerConfig.hpp"

class Server 
{
	public:
		// Constructor
		Server(void);
		Server(const Server &obj);
		Server(const ServerConfig &config);

		// Operator
		Server &operator=(const Server &obj);

		// Destructor
		~Server(void);
		
		// Getters
		int getServerFD(void) const;
		const ServerConfig &getConfig(void) const;
		
		// Others
		void initSocket(void);

	private: 
		int _serverFD;
		ServerConfig _config;
	
};

#endif