#include "../../includes/RequestDispatcher.hpp"

RequestDispatcher::RequestDispatcher(void) {}
RequestDispatcher::RequestDispatcher(const RequestDispatcher &obj) { (void)obj; }
RequestDispatcher &RequestDispatcher::operator=(const RequestDispatcher &obj)
{
	(void) obj;
	return (*this);
}

RequestDispatcher::~RequestDispatcher() {}
		IRequestHandler *RequestDispatcher::selectHandler(const Request &req, const Route &route) const
{
	// if (route.isRedirect)
	// 	return (new RedirectHandler(route));
	// else if (isCGIRequest(req.getTarget(), route));
	// 	return (new CGIHandler(route));
	// else if (isUploadRequest(req.getMethod(), route))
	// 	return (new UploadHandler(route));
	(void) req;
	return (new StaticFileHandler(route));
}
