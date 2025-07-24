#ifndef REDIRECTHANDLER_HPP
#define REDIRECTHANDLER_HPP

#include "common.hpp"
#include "IRequestHandler.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Route.hpp"
#include "ServerConfig.hpp"

class RedirectHandler : public IRequestHandler
{
	public:
		RedirectHandler(const Route &route, const ServerConfig &config);
		virtual ~RedirectHandler(void);

		virtual void handle(const Request &req, Response &res);

	private:
		const Route &_route;
		const ServerConfig &_config;

		RedirectHandler();
		RedirectHandler(const RedirectHandler &obj);
		RedirectHandler &operator=(const RedirectHandler &obj);
};

#endif