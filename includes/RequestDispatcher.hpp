#ifndef REQUESTDISPATCHER_HPP
#define REQUESTDISPATCHER_HPP

#include "IRequestHandler.hpp"
#include "Request.hpp"
#include "Route.hpp"
#include "StaticFileHandler.hpp"

class RequestDispatcher
{
	public:
		// Constructor
		RequestDispatcher(void);
		RequestDispatcher(const RequestDispatcher &obj);

		// Operator
		RequestDispatcher &operator=(const RequestDispatcher &obj);
		
		// Destructor
		~RequestDispatcher();
		
		// Others
		IRequestHandler *selectHandler(const Request &req, const Route &route) const;
};
	
#endif