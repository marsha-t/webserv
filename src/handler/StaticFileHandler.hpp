#ifndef STATICFILEHANDLER_HPP
#define STATICFILEHANDLER_HPP

#include "../../includes/common.hpp"
#include "IRequestHandler.hpp"
#include "../http/Request.hpp"
#include "../http/Response.hpp"
#include "../config/Route.hpp"


class StaticFileHandler: public IRequestHandler
{
	public:
		// Constructor
		StaticFileHandler(void);
		StaticFileHandler(const StaticFileHandler &obj);
		StaticFileHandler(const Route &route);
		
		// Operator 
		StaticFileHandler &operator=(const StaticFileHandler &obj);
		
		// Destructor
		virtual ~StaticFileHandler(void);
		
		// Others
		virtual void handle (const Request &req, Response &res);
	private:
		Route _route;
		
		bool isSafePath(const std::string &path) const;
		bool fileExists(const std::string &path, const std::string &method) const;
		bool readFile(const std::string &path, std::string &content) const;
		std::string getMimeType(const std::string &filename) const;
};

#endif