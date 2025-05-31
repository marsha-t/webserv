#include "ConfigParser.hpp"

// Constructors don't parse (since parse() throws an exception)
ConfigParser::ConfigParser(void) {}
ConfigParser::ConfigParser(const std::string &filename): _filename(filename) {}
ConfigParser::ConfigParser(const ConfigParser &obj): _filename(obj._filename), _servers(obj._servers) {}
ConfigParser::~ConfigParser(void) {}
ConfigParser &ConfigParser::operator=(const ConfigParser &obj)
{
	if (this != &obj)
	{
		_filename = obj._filename;
		_servers = obj._servers;
	}
	return (*this);
}

const std::vector<ServerConfig> &ConfigParser::getServers(void) const { return _servers; }

void	ConfigParser::parse(void)
{
	std::ifstream file(_filename.c_str());
	if (!file.is_open())
	{
		throw std::runtime_error("Unable to open configuration file: " + _filename);
	}
	std::string line;
	while (std::getline(file, line))
	{
		line = cleanLine(line);
		if (line.empty())
			continue;
		if (line == "server {")
			parseServerBlock(file);
		else
			throw std::runtime_error("Expected 'server {' but got : " + line);
	}
}

/// Removes comments, leading & trailing whitespace and trailing semicolons
std::string ConfigParser::cleanLine(const std::string &line)
{
	std::string trimmed = line;
	size_t comment = trimmed.find('#');
	if (comment != std::string::npos)
		trimmed = trimmed.substr(0, comment);
	while (!trimmed.empty() && (trimmed.back() == ' ' || trimmed.back() == '\t' || trimmed.back() == ';'))
		trimmed.pop_back();
	size_t start = trimmed.find_first_not_of(" \t");
	if (start != std::string::npos)
		trimmed = trimmed.substr(start);
	return trimmed;
}

/// Checks
/// - server block has opening and closing brace
/// - location block is started with curly brace on same line or next line

/// Defaults: 0.0.0.0 if no host specified; 80 if no port specified (like in NGINX)
void	ConfigParser::parseServerBlock(std::istream &in)
{
	ServerConfig server;
	std::string line;
	bool closingBrace = false;
	bool hostSet = false;
	bool portSet = false;

	while (std::getline(in, line))
	{
		line = cleanLine(line);
		if (line.empty())
			continue;
		if (line == "}")
		{
			closingBrace = true;
			break ;
		}
		std::vector<std::string> tokens = tokenize(line);
		if (tokens.empty())
			continue;
		if (tokens[0] == "host" && tokens.size() == 2)
		{
			if (hostSet)
				throw std::runtime_error("Duplicate 'host' directive in server block");
			server.setHost(tokens[1]);
			hostSet = true;
		}
		else if (tokens[0] == "port" && tokens.size() == 2)
		{
			if (portSet)
				throw std::runtime_error("Duplicate 'port' directive in server block");
			server.setPort(std::atoi(tokens[1].c_str()));
			portSet = true;
		}
		else if (tokens[0] == "server_name" && tokens.size() >= 2)
		{
			for (size_t i = 1; i < tokens.size(); ++i)
				server.addServerName(tokens[1]); // change server from set to add
		}
		else if (tokens[0] == "error_page" && tokens.size() == 3)
			server.addErrorPage(std::atoi(tokens[1].c_str()));
		else if (tokens[0] == "client_max_body_size" && tokens.size() == 2)
			server.setClientMaxBodySize(std::atoi(tokens[1].c_str()));
		else if (tokens[0] == "location" && tokens[1][0] == '/')
		{
			Route route;
			route.setLocation(tokens[1]);
			if (tokens.size() == 3 && tokens[2] == "{")
			{
			}
			else if (tokens.size() == 2)
			{
				std::string nextLine;
				if (!std::getline(in, nextLine))
					throw std::runtime_error("Unexpected end of file after location " + tokens[1]);
				if (cleanLine(nextLine) != "{")
					throw std::runtime_error("Expected '{' after location " + tokens[1]);
			}
			else
				throw std::runtime_error("Unknown or malformed directive in server block " + line);
			parseLocationBlock(in, route);
			server.addRoute(route);
		}
		else
			throw std::runtime_error("Unknown or malformed directive in server block " + line);
	}
	if (!hostSet)
		server.setHost("0.0.0.0");
	if (!portSet)
		server.setPort(80);
	if (!closingBrace)
		throw std::runtime_error("Missing closing '}' for server block");
	_servers.push_back(server);
}
