#include "../../includes/RedirectHandler.hpp"

RedirectHandler::RedirectHandler(const Route &route, const ServerConfig &config) : _route(route), _config(config) {}

RedirectHandler::~RedirectHandler(void) {}

void RedirectHandler::handle(const Request &req, Response &res)
{
	(void)req;
	res.setStatusLine(_route.getRedirectStatusCode(), "Moved Permanently"); // TODO: message based on status code
	res.setHeader("Location", _route.getRedirectURL());
}
