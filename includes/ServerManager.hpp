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
		ServerManager(const std::vector<ServerConfig> &configs);
		ServerManager(const ServerManager &obj);
		
		// Destructor
		~ServerManager(void);
		
		// Operators
		ServerManager &operator=(const ServerManager &obj);
		
		// Others
		void	setup(void);
		void	start(void);
		
	private:
		std::vector<Server> _servers;
		std::map<int, const Server*> _clientToServer; // map each client connection FD after accept to Server that accepted it; needed to select right ServerConfig later
		std::map<int, std::string> _clientBuffers; // raw data received from each client
		std::map<int, Request> _clientRequests; // parsed request for each clientFD
		std::map<int, std::string> _clientResponses; // pending response body
		std::map<int, size_t> _responseOffsets; // How much has been sent 
		fd_set _master_read_fds;
		fd_set _master_write_fds;
		int _max_fd;

		void	selectLoop(void);
		bool	isListeningSocket(int fd) const;
		void	acceptNewClient(int serverFD);
		bool	handleClientRead(int fd, int &parseError, Request *tempReq);
		void	sendErrorResponse(int fd, int errorCode, const Request& request);
		bool	readFromClient(int fd, std::string& buffer, int& parseError);
		bool	headersComplete(const std::string& buffer);
		bool	parseTempHeaders(const std::string& buffer, Request& tempReq);
		bool	isBodyComplete(const std::string& buffer, const Request& tempReq);
		bool	parseFullRequest(const std::string& buffer, Request& requestOut);
		void	processClientRequest(int clientFD, Request& request);
		const	ServerConfig* getSelectedConfig(int clientFD, const Request& request);
		bool	matchRouteOrRespond404(int clientFD, const Request& request, const ServerConfig& config, Route& matchedRoute);
		std::size_t	getMaxBodySize(const Route& route, const ServerConfig& config);
		bool	validateRequestOrRespondError(int clientFD, Request& request, const ServerConfig& config, std::size_t maxBodySize);
		void	generateResponseAndBuffer(int clientFD, const Request& request, const Route& route, const ServerConfig& config);
		void	bufferResponse(int fd, const std::string& responseStr);
		void	handleClientWrite(int fd);
		const Server*	getListeningServerByFD(int fd) const;
		void	cleanupClient(int fd);
		void	trackMaxFd(int fd);

};

#endif