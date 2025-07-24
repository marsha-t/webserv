#ifndef REQUESTDISPATCHER_HPP
#define REQUESTDISPATCHER_HPP

#include "IRequestHandler.hpp"
#include "Request.hpp"
#include "Route.hpp"
#include "StaticFileHandler.hpp"
#include "CgiHandler.hpp"
#include "UploadHandler.hpp"
#include "RedirectHandler.hpp"
#include "FormHandler.hpp"
#include "ServerConfig.hpp"

class RequestDispatcher
{
	public:
		// Constructor
		RequestDispatcher(void);
		
		// Destructor
		~RequestDispatcher();
		
		// Others
		IRequestHandler *selectHandler(const Request &req, const Route &route, const ServerConfig &config) const;
	
	private:
		std::string getFileExtension(const std::string &path) const;
		bool isCgiRequest(const Request &req, const Route &route) const;
		
		RequestDispatcher(const RequestDispatcher &obj);
		RequestDispatcher &operator=(const RequestDispatcher &obj);

};
	
#endif