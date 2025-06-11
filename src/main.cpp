#include "../includes/common.hpp"
#include "../includes/Request.hpp"
#include "../includes/ConfigParser.hpp"
#include "../includes/Route.hpp"
#include "../includes/RequestDispatcher.hpp"
#include "../includes/IRequestHandler.hpp"
#include "../includes/StaticFileHandler.hpp"
#include "../includes/Server.hpp"

void    errorMsg(std::string msg)
{
	std::cerr << RED << msg << RESET << std::endl;
}

int	test(void)
{
	// Create listening socket
	int	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
	{
		errorMsg("Failed to create listening socket");
		return (1);
	}

	// Allow address to be reused
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		errorMsg("Failed to allow address reuse");
		close(server_fd);
		return (1);
	}
	
	// Bind socket to port 8080 on localhost
	sockaddr_in address;
	int port = 8080;
	std::memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	if (bind(server_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0)
	{
		errorMsg("Failed to bind socket");
		close(server_fd);
		return (1);
	}

	// Listen on socket
	if (listen(server_fd, SOMAXCONN) < 0) // set to max allowed backlog before server refuses connections 
	{
		errorMsg("Failed to listen on socket");
		close(server_fd);
		return (1);
	}
	
	std::cout << "Listening on " << port << std::endl;

	// Accept connection
    sockaddr_in client_address;
	socklen_t	client_len = sizeof(client_address);
    int client_socket = accept(server_fd, reinterpret_cast<struct sockaddr*>(&client_address), &client_len);
    if (client_socket < 0)
    {
        errorMsg("Failed to accept connection");
        close(server_fd);
        return (1);
    }

    std::cout << GREEN "Connection accepted on " << port << RESET << std::endl;

	// 5. Read the HTTP request (up to 1024 bytes)
    std::string	rawRequest;
	char buffer[1024];
    std::memset(buffer, 0, sizeof(buffer));
	int bytes_read;
	while ((bytes_read = read(client_socket, buffer, sizeof(buffer) - 1)) > 0)
	{
		buffer[bytes_read] = '\0';
		rawRequest += buffer;
		if (rawRequest.find("\r\n\r\n") != std::string::npos)
			break;
	}

	Request req;
	if (!req.parse(rawRequest))
	{
		errorMsg("Failed to parse HTTP request");
		return (1); // Return 400 Bad Request
	}
	
	Route dummyRoute;
	dummyRoute.setRoot("www");
	dummyRoute.setAutoindex(false);
	dummyRoute.addMethod("GET");
	dummyRoute.addIndexFile("index.html");
	Response res;
	RequestDispatcher dispatcher;
	IRequestHandler *handler = dispatcher.selectHandler(req, dummyRoute);
	handler->handle(req, res);
	delete handler;

    write(client_socket, res.toString().c_str(), res.toString().size());

	close(server_fd);
	close(client_socket);

	return (0);
}

// Use config file or default config
// Parse config file using ConfigParser to create ServerConfig
// Create sockets for each Server
// Implement poll loop
// Implement client connction logic
// Parse Request
// Route to file
// Build Response

int	main(int argc, char **argv)
{
	// (void) argc;
	// (void) argv;
	// test();
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
		// std::vector<Server> servers;
		// for (size_t i = 0; i < configs.size(); ++i)
		// {
		// 	Server indivServer(configs[i]);
		// 	indivServer.initSocket();
		// 	servers.push_back(indivServer);
		// }
		// // pollLoop(server); // handles requests etc
		// while (true)
		// 	pause();
	}
	catch (std::exception &e)
	{
		errorMsg(e.what());
		return (1);
	}
	return (0);
}