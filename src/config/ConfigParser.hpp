#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "../../includes/common.hpp"
#include "ServerConfig.hpp"

class ConfigParser
{
	public:
		// Constructor 
		ConfigParser(void);
		ConfigParser(const std::string &filename);
		ConfigParser(const ConfigParser &obj);
		
		// Destructor
		~ConfigParser(void);

		// Operator
		ConfigParser &operator=(const ConfigParser &obj);

		// Getters
		const std::vector<ServerConfig> &getServers(void) const;

		void	parse(void);

	private:
		std::string _filename;
		std::vector<ServerConfig> _servers;
		
		std::string cleanLine(const std::string &line);
		void	parseServerBlock(std::istream &in);
		std::vector<std::string> tokenize(const std::string &line);
		void	parseHostDirective(ServerConfig &server, const std::vector<std::string> &tokens, bool &hostSet);
		void	parsePortDirective(ServerConfig &server, const std::vector<std::string> &tokens, bool &portSet);
		void	parseServerName(ServerConfig &server, const std::vector<std::string> &tokens);
		void 	parseErrorPage(ServerConfig &server, const std::vector<std::string> &tokens);
		void	parseLocation(ServerConfig &server, std::istream &in, const std::vector<std::string> &tokens);
		void	parseLocationBlock(std::istream &in, Route &route);
		void	parseCGIBlock(std::istream &in, Route &route);

};

#endif
