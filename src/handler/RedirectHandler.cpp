#include "../../includes/RedirectHandler.hpp"

RedirectHandler::RedirectHandler(const Route &route, const ServerConfig &config) : _route(route), _config(config) {}

RedirectHandler::~RedirectHandler(void) {}

void RedirectHandler::handle(const Request &req, Response &res)
{
	(void)req;
	std::string url = _route.getRedirectURL();
	if (url.empty()) {
		res.setError(500, _config);
		return;
	}
	res.setHeader("Location", url);

	int code = _route.getRedirectStatusCode();
	if (code < 300 || code >= 400)
		code = 302;
	res.setStatusLine(code, httpStatusMessage(code));
}
