#include "../../includes/RequestDispatcher.hpp"

RequestDispatcher::RequestDispatcher(void) {}
RequestDispatcher::RequestDispatcher(const RequestDispatcher &obj) { (void)obj; }
RequestDispatcher &RequestDispatcher::operator=(const RequestDispatcher &obj)
{
	(void) obj;
	return (*this);
}

RequestDispatcher::~RequestDispatcher() {}

IRequestHandler *RequestDispatcher::selectHandler(const Request &req, const Route &route, const ServerConfig &config) const
{
	if (!route.isMethodAllowed(req.getMethod()))
	{
		debugMsg("405 Method Not Allowed for " + req.getMethod());
		return NULL;
	}

	if (route.isRedirect())
	{
		debugMsg("Selecting RedirectHandler");
		return (new RedirectHandler(route, config));
	}
	if (req.getMethod() == "POST" && !route.getUploadDir().empty())
	{
		debugMsg("Selecting UploadHandler");
		return (new UploadHandler(route, config));
	}
	if (isCgiRequest(req, route))
	{
		std::string ext = toLower(getFileExtension(req.getTarget()));
		debugMsg("Selecting CgiHandler for extension: " + ext);
		return new CgiHandler(route, config);
	}
	debugMsg("Selecting StaticFileHandler");
	return (new StaticFileHandler(route, config));
}

std::string RequestDispatcher::getFileExtension(const std::string &path) const
{
	std::string::size_type dotPos = path.find_last_of(".");
	if (dotPos != std::string::npos)
		return path.substr(dotPos);
	return "";
}

bool RequestDispatcher::isCgiRequest(const Request &req, const Route &route) const
{
	std::string ext = toLower(getFileExtension(req.getTarget()));
	return route.getCGI().count(ext) > 0;
}
