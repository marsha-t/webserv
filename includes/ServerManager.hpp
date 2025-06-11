#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include "common.hpp"
#include "ServerConfig.hpp"
#include "Server.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "RequestDispatcher.hpp"

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
		std::map<int, const Server*> _clientToServer;
		std::map<int, std::string> _clientBuffers;
		std::map<int, Request> _clientRequests;
		// TODO to support partial writes (writing in chunks and resume later), and
		// 		persistent connections (Connection: keep-alive)
		// std::map<int, Response> _clientResponses
		
		// TODO to track which connections should close after response is fully sent
		// std::set<int> _closingClients;
			
		bool isListeningSocket(int fd) const;
		void pollLoop(std::vector<struct pollfd> &fds);
		void acceptNewClient(int serverFD, std::vector<struct pollfd> &fds);
		bool handleClientRead(int clientFD, Request &requestOut);
		void processClientRequest(int clientFD, const Request &request);
		const Server* getServerByFD(int fd) const;
		void cleanupClient(int fd, std::vector<struct pollfd> &fds, size_t &i);


};

#endif