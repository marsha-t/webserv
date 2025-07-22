#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "common.hpp"
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
		void	handleRootDirective(const std::vector<std::string>& tokens, Route& route, bool& rootSet);
		void	handleIndexDirective(const std::vector<std::string>& tokens, Route& route);
		void	handleAutoindexDirective(const std::vector<std::string>& tokens, Route& route, bool& autoindexSet);
		void	handleMethodsDirective(const std::vector<std::string>& tokens, Route& route);
		void	handleReturnDirective(const std::vector<std::string>& tokens, Route& route, bool& redirectSet);
		void	handleUploadDirDirective(const std::vector<std::string>& tokens, Route& route, bool& uploadDirSet);
		void	handleClientMaxBodyDirective(const std::vector<std::string>& tokens, Route& route);
		void	handleCGIDirective(const std::vector<std::string>& tokens, Route& route);

};

#endif
