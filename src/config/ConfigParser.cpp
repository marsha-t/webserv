#include "../../includes/ConfigParser.hpp"

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
		throw std::runtime_error("Unable to open configuration file: " + _filename);
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
	std::string::size_type comment = trimmed.find('#');
	if (comment != std::string::npos)
		trimmed = trimmed.substr(0, comment);
	while (!trimmed.empty() && (trimmed[trimmed.size() - 1] == ' ' || trimmed[trimmed.size() - 1] == '\t' || trimmed[trimmed.size() - 1] == ';'))
		trimmed.erase(trimmed.size() - 1);
	std::string::size_type start = trimmed.find_first_not_of(" \t");
	if (start != std::string::npos)
		trimmed = trimmed.substr(start);
	return trimmed;
}

/// Checks
/// - server block has opening and closing brace
/// - location block is started with curly brace on same line or next line
/// No checks for configuration extension (NGINX doesn't require one, though .conf is a convention)
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
		if (tokens[0] == "host")
			parseHostDirective(server, tokens, hostSet);
		else if (tokens[0] == "port")
			parsePortDirective(server, tokens, portSet);
		else if (tokens[0] == "server_name")
			parseServerName(server, tokens);
		else if (tokens[0] == "error_page")
			parseErrorPage(server, tokens);
		else if (tokens[0] == "client_max_body_size" && tokens.size() == 2)
			server.setClientMaxBodySize(std::atoi(tokens[1].c_str()));
		else if (tokens[0] == "location")
			parseLocation(server, in, tokens);
		else
			throw std::runtime_error("Unknown or malformed directive in server block: " + line);
	}
	if (!hostSet)
		server.setHost("0.0.0.0");
	if (!portSet)
		server.setPort(80);
	if (!closingBrace)
		throw std::runtime_error("Missing closing '}' for server block");
	_servers.push_back(server);
}

std::vector<std::string> ConfigParser::tokenize(const std::string &line)
{
	std::istringstream iss(line);
	std::string token;
	std::vector<std::string> tokens;

	while (iss >> token)
		tokens.push_back(token);

	return tokens;
}

void ConfigParser::parseHostDirective(ServerConfig &server, const std::vector<std::string> &tokens, bool &hostSet)
{
	if (tokens.size() != 2)
		throw std::runtime_error("Invalid number of arguments for 'host'");
	if (hostSet)
		throw std::runtime_error("Duplicate 'host' directive");
	server.setHost(tokens[1]);
	hostSet = true;
}

void ConfigParser::parsePortDirective(ServerConfig &server, const std::vector<std::string> &tokens, bool &portSet)
{
	if (tokens.size() != 2)
		throw std::runtime_error("Invalid number of arguments for 'port'");
	if (portSet)
		throw std::runtime_error("Duplicate 'port' directive");
	server.setPort(std::atoi(tokens[1].c_str()));
	portSet = true;
}

void ConfigParser::parseServerName(ServerConfig &server, const std::vector<std::string> &tokens)
{
	if (tokens.size() < 2)
		throw std::runtime_error("Missing argument for 'server_name'");
	for (size_t i = 1; i < tokens.size(); ++i)
		server.addServerName(tokens[i]);
}

void ConfigParser::parseErrorPage(ServerConfig &server, const std::vector<std::string> &tokens)
{
	if (tokens.size() != 3)
		throw std::runtime_error("Invalid 'error_page' directive");
	int code = std::atoi(tokens[1].c_str());
	server.addErrorPage(code, tokens[2]);
}

void ConfigParser::parseLocation(ServerConfig &server, std::istream &in, const std::vector<std::string> &tokens)
{
	if (tokens.size() < 2 || tokens[1][0] != '/')
		throw std::runtime_error("Malformed 'location' directive");

	Route route;
	route.setLocation(tokens[1]);

	if (tokens.size() == 3 && tokens[2] == "{")
	{
	}
	else if (tokens.size() == 2)
	{
		std::string nextLine;
		if (!std::getline(in, nextLine))
			throw std::runtime_error("Unexpected EOF after 'location' " + tokens[1]);
		if (cleanLine(nextLine) != "{")
			throw std::runtime_error("Expected '{' after location " + tokens[1]);
	}
	else
		throw std::runtime_error("Malformed 'location' directive");

	parseLocationBlock(in, route);
	server.addRoute(route);
}

void ConfigParser::parseLocationBlock(std::istream &in, Route &route)
{
	std::string line;
	bool closingBrace = false;
	bool rootSet = false;
	bool autoindexSet = false;
	bool redirectSet = false;
	bool uploadDirSet = false;
	
	while (std::getline(in, line))
	{
		line = cleanLine(line);
		if (line.empty())
			continue;
		if (line == "}")
		{
			closingBrace = true;
			break;
		}

		std::vector<std::string> tokens = tokenize(line);
		if (tokens[0] == "root" && tokens.size() == 2)
		{
			if (rootSet)
				throw std::runtime_error("Duplicate 'root' directive in location block");
			route.setRoot(tokens[1]);
			rootSet = true;
		}
		else if (tokens[0] == "index" && tokens.size() >= 2)
		{
			for (size_t i = 1; i < tokens.size(); ++i)
				route.addIndexFile(tokens[i]);
		}
		else if (tokens[0] == "autoindex" && tokens.size() == 2)
		{
			if (autoindexSet)
				throw std::runtime_error("Duplicate 'autoindex' directive in location block");
			if (tokens[1] == "on")
				route.setAutoindex(true);
			else if (tokens[1] == "off")
				route.setAutoindex(false);
			else
				throw std::runtime_error("Invalid value for 'autoindex': " + tokens[1]);
			autoindexSet = true;
		}
		else if (tokens[0] == "methods" && tokens.size() >= 2)
		{
			for (size_t i = 1; i < tokens.size(); ++i)
				route.addAllowedMethod(tokens[i]);
		}
		else if (tokens[0] == "return" && tokens.size() == 3)
		{
			if (redirectSet)
				throw std::runtime_error("Duplicate 'return' directive in location block");
			int statusCode = std::atoi(tokens[1].c_str());
			route.setRedirect(statusCode, tokens[2]);
			redirectSet = true;
		}
		else if (tokens[0] == "upload_dir" && tokens.size() == 2)
		{
			if (uploadDirSet)
				throw std::runtime_error("Duplicate 'upload_dir' directive in location block");
			route.setUploadDir(tokens[1]);
			uploadDirSet = true;
		}
		else if (tokens[0] == "client_max_body_size" && tokens.size() == 2)
		{
			route.setClientMaxBodySize(std::atoi(tokens[1].c_str()));
		}
		else if (tokens[0] == "cgi" && tokens.size() == 3)
		{
			route.addCGI(tokens[1], tokens[2]);
		}
		else
			throw std::runtime_error("Unknown or malformed directive in location block: " + line);
	}
	
	if (!closingBrace)
		throw std::runtime_error("Missing closing '}' in location block for path: " + route.getLocation());
}



