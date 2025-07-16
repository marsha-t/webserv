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

		std::string sanitizeFilename(const std::string& filename);
		bool saveFile(const std::string &filename, const std::string &content) const;
		bool isSafePath(const std::string &path, const std::string &baseDir) const;

		UploadHandler();
		UploadHandler &operator=(const UploadHandler &obj);
};

#endif