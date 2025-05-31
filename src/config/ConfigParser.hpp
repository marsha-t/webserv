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
};

#endif


// Helpers
		void	parseLocationBlock(std::istream &inStream, Route &route);
		void	skipEmptyLinesAndComments(std::istream &inStream);

		// Tokenizing and validation
		std::vector<std::string> tokenize(const std::string &line);
		bool	isValidDirective(const std::string &directive);