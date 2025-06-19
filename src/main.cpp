#include "../includes/common.hpp"
#include "../includes/Request.hpp"
#include "../includes/ConfigParser.hpp"
#include "../includes/Route.hpp"
#include "../includes/RequestDispatcher.hpp"
#include "../includes/IRequestHandler.hpp"
#include "../includes/StaticFileHandler.hpp"
#include "../includes/Server.hpp"
#include "../includes/ServerManager.hpp"

void    errorMsg(std::string msg)
{
	std::cerr << RED << msg << RESET << std::endl;
}

int	main(int argc, char **argv)
{
	try
	{
		if (argc > 2)
			throw std::runtime_error("Usage: ./webserv [config_file]");
			
		std::string configFile = (argc == 1 ? "config/default.conf" : argv[1]);
		ConfigParser parser(configFile);
		parser.parse();
		std::vector<ServerConfig> configs = parser.getServers();
		ServerManager manager(configs);
		manager.setup();
		manager.start();
	}
	catch (std::exception &e)
	{
		errorMsg(e.what());
		return (1);
	}
	return (0);
}