#ifndef IREQUESTHANDLER_HPP
#define IREQUESTHANDLER_HPP

#include "Request.hpp"
#include "Response.hpp"
class IRequestHandler 
{
	public:
		virtual ~IRequestHandler(void) {}
		virtual void handle(const Request &request, Response &response) = 0;
};

#endif