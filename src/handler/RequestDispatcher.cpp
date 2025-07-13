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
	if (route.isRedirect())
	{
		std::cerr << "Selecting RedirectHandler" << std::endl;
		return (new RedirectHandler(route, config));
	}
	if (req.getMethod() == "POST" && !route.getUploadDir().empty())
	{
		std::cerr << "Selecting UploadHandler" << std::endl;
		return (new UploadHandler(route, config));
	}

	std::string target = req.getTarget();
	std::string::size_type dotPos = target.find_last_of(".");
	if (dotPos != std::string::npos)
	{
		std::string ext = target.substr(dotPos);
		if (route.getCGI().count(ext))
		{
			std::cerr << "Selecting CgiHandler for extension: " << ext << std::endl;
			return (new CgiHandler(route, config));
		}
	}
	std::cerr << "Selecting StaticFileHandler" << std::endl;
	return (new StaticFileHandler(route, config));
}
