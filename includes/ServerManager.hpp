#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include "common.hpp"
#include "ServerConfig.hpp"

class ServerManager
{
	public:
		// Constructor
		ServerManager(void);
		ServerManager(const ServerManager &obj);
		ServerManager(const std::vector<ServerConfig> &configs);
		
		// Destructor
		~ServerManager(void);
		
		// Operators
		ServerManager &operator=(const ServerManager &obj);
		
		// Getters
		
		// Others
		void	setup(void);
		void	start(void);
		
	private:
		std::vector<Server> _servers;
		std::vector<int> _serverFDs;
		std::map<int, const Server*> _clientToServer;
		std::map<int, std::string> _clientBuffers;
		std::map<int, Request> _clientRequests;
		std::map<int, Response> _clientResponses;
		std::set<int> _closingClients;
			
		bool isListeningSocket(int fd) const;
		void acceptNewClient(int clientFD, std::vector<pollfd>& fds)
		bool handleClientRead(int clientFD);
		const Server* getServerByFD(int fd) const;

		void acceptNewClient(int serverFD, std::vector<struct pollfd> &fds);
		bool handleClientRead(int clientFD, Request &requestOut);
		void processClientRequest(int clientFD, const Request &request);


};

#endif