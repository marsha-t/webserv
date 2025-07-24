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
		return new RedirectHandler(route, config);
	}
	if (isCgiRequest(req, route) && (req.getMethod() == "GET" || req.getMethod() == "POST"))
	{
		std::string ext = toLower(getFileExtension(req.getTarget()));
		debugMsg("Selecting CgiHandler for extension: " + ext);
		return new CgiHandler(route, config);
	}
	if (req.getMethod() == "POST" && !route.getUploadDir().empty() && req.getHeader("content-type").find("multipart/form-data") != std::string::npos)
	{
		debugMsg("Selecting UploadHandler");
		return new UploadHandler(route, config);
	}
	if (req.getMethod() == "POST" && req.getHeader("content-type").find("application/x-www-form-urlencoded") != std::string::npos)
	{
		debugMsg("Selecting FormHandler for x-www-form-urlencoded");
		return new FormHandler(route, config);
	}
	if (req.getMethod() == "GET" || req.getMethod() == "DELETE")
	{
		debugMsg("Selecting StaticFileHandler");
		return new StaticFileHandler(route, config);
	}
	debugMsg("405 Method Not Allowed: No handler for " + req.getMethod() + " on target " + req.getTarget());
	return NULL;
}


std::string RequestDispatcher::getFileExtension(const std::string &path) const
{
	std::string::size_type queryPos = path.find('?');
	std::string cleanPath = (queryPos != std::string::npos) ? path.substr(0, queryPos) : path;

	std::string::size_type dotPos = cleanPath.find_last_of(".");
	if (dotPos != std::string::npos)
		return cleanPath.substr(dotPos);
	return "";
}

bool RequestDispatcher::isCgiRequest(const Request &req, const Route &route) const
{
	std::string ext = toLower(getFileExtension(req.getTarget()));
	return route.getCGI().count(ext) > 0;
}
