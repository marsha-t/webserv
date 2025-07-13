#ifndef UPLOADHANDLER_HPP
#define UPLOADHANDLER_HPP

#include "IRequestHandler.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Route.hpp"
#include "ServerConfig.hpp"

class UploadHandler : public IRequestHandler
{
	public:
		UploadHandler(const Route &route, const ServerConfig &config);
		virtual ~UploadHandler(void);

		virtual void handle(const Request &req, Response &res);

	private:
		const Route &_route;
		const ServerConfig &_config;

		bool saveFile(const std::string &filename, const std::string &content) const;
};

#endif