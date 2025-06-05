#ifndef REQUESTDISPATCHER_HPP
#define REQUESTDISPATCHER_HPP

#include "IRequestHandler.hpp"
#include "../http/Request.hpp"
#include "../config/Route.hpp"
#include "StaticFileHandler.hpp"

class RequestDispatcher
{
	public:
		IRequestHandler *selectHandler(const Request &req, const Route &route) const;
};
	
#endif