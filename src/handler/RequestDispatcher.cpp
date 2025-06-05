#include "RequestDispatcher.hpp"

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
