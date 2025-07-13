#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include "IRequestHandler.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Route.hpp"
#include "ServerConfig.hpp"

class CgiHandler : public IRequestHandler
{
	public:
		CgiHandler(const Route &route, const ServerConfig &config);
		virtual ~CgiHandler(void);

		virtual void handle(const Request &req, Response &res);

	private:
		const Route &_route;
		const ServerConfig &_config;

		std::map<std::string, std::string> getCgiEnv(const Request &req) const;
		std::string executeCgi(const Request &req, Response &res, const std::string &scriptPath) const;
};

#endif