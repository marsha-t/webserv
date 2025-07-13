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
		std::map<int, const Server*> _clientToServer; // map each client connection FD after accept to Server that accepted it; needed to select right ServerConfig later
		std::map<int, std::string> _clientBuffers; // raw data received from each client
		std::map<int, Request> _clientRequests; // parsed request for each clientFD
		
		// TODO to support partial writes (writing in chunks and resume later), and
		// 		persistent connections (Connection: keep-alive)
		// std::map<int, Response> _clientResponses
		
		// TODO to track which connections should close after response is fully sent
		// std::set<int> _closingClients;
			
		bool isListeningSocket(int fd) const;
		void	selectLoop(void);
		void	acceptNewClient(int serverFD, fd_set &master_fds, int &max_fd);
		bool	handleClientRead(int clientFD);

		void	processClientRequest(int clientFD, Request &request);
		const Server*	getListeningServerByFD(int fd) const;
		void	cleanupClient(int fd, fd_set &master_fds);
};

#endif