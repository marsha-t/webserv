#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include "../../includes/common.hpp"
#include "Route.hpp"

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
		void	addServerName(const std::string &serverName);
		void	addErrorPage(int code, const std::string &filepath);
		void	addRoute(const Route &route);
		void	setClientMaxBodySize(unsigned int clientMaxBodySize);
		// Getters
		const std::string &getHost(void) const;
		int	getPort(void) const;
		const std::vector<std::string> &getServerNames(void) const;
		const std::vector<Route> &getRoutes(void) const;
		const std::map<int, std::string> &getErrorPages(void) const;
		unsigned int getClientMaxBodySize(void) const;

	private:
		std::string	_host;
		int	_port;
		std::vector<std::string> _serverNames;
		std::map<int, std::string> _errorPages;
		std::vector<Route> _routes;
		unsigned int _clientMaxBodySize;

};

#endif