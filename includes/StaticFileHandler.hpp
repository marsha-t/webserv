#ifndef STATICFILEHANDLER_HPP
#define STATICFILEHANDLER_HPP

#include "common.hpp"
#include "IRequestHandler.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Route.hpp"
#include "ServerConfig.hpp"


class StaticFileHandler: public IRequestHandler
{
	public:
		// Constructor
		StaticFileHandler(const StaticFileHandler &obj);
		StaticFileHandler(const Route &route, const ServerConfig &config);
		
		// Destructor
		virtual ~StaticFileHandler(void);
		
		// Others
		virtual void handle (const Request &req, Response &res);
	private:
		Route _route;
		const ServerConfig &_config;
		
		bool isMethodAllowed(const Request &req) const;
		std::string resolvePath(const Request &req) const;
		bool isSafePath(const std::string &path) const;
		bool handleDirectory(const Request &req, Response &res, std::string &path) const;
		bool handleDelete(Response &res, const std::string &path) const;
		bool serveFile(Response &res, const std::string &path) const;
		bool findIndexFile(const std::string &dir, std::string &indexPath) const;
		bool readFile(const std::string &path, std::string &content) const;
		std::string getMimeType(const std::string &filename) const;
		std::string generateDirectoryListing(const std::string &dirPath, const std::string &uriPath) const;
		
		StaticFileHandler();
		StaticFileHandler &operator=(const StaticFileHandler &obj);
};

#endif