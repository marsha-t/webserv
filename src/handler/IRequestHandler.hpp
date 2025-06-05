#ifndef IREQUESTHANDLER_HPP
#define IREQUESTHANDLER_HPP

#include "../http/Request.hpp"
#include "../http/Response.hpp"
class IRequestHandler 
{
	public:
		virtual ~IRequestHandler(void) {}
		virtual void handle(const Request &request, Response &response) = 0;
};

#endif