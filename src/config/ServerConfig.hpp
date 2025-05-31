#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include "../../includes/common.hpp"

class ServerConfig
{
	public:
		// Constructor
		ServerConfig(void);
		ServerConfig(const ServerConfig &obj);
		
		// Destructor
		~ServerConfig(void);
		
		// Operator
		ServerConfig &operator=(const ServerConfig &obj);

		// Setters
		void	setHost(const std::string &host);
		void	setPort(int port);
		void	setServerName(const std::string &serverName);
		void	addErrorPage(int code, const std::string &filepath);
		void	addRoute(const Route &route);
		void	addClientMaxBodySize(unsigned int clientMaxBodySize);
		// Getters
		const std::string &getHost(void) const;
		int	getPort(void) const;
		const std::string &getServerName(void) const;
		const std::vector<Route> &getRoutes(void) const;
		const std::map<int, std::string> &getErrorPages(void) const;
		unsigned int getClientMaxBodySize(void) const;

	private:
		std::string	_host;
		int	_port;
		std::string _serverName;
		std::map<int, std::string> _errorPages;
		std::vector<Route> _routes;
		unsigned int _clientMaxBodySize;

};

#endif