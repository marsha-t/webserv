#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include "common.hpp"
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
		char **createEnvArray(const std::map<std::string, std::string>& envMap) const;
		void handleChildProcess(const Request &req, const std::string &scriptPath, char **env, int pipe_in[2], int pipe_out[2]) const;
		bool writeRequestBody(const Request &req, int writeFd) const;
		std::string readCgiOutput(int readFd) const;
		void parseCgiResponse(const std::string &cgiOutput, Response &res) const;

		CgiHandler();
		CgiHandler(const CgiHandler &obj);
		CgiHandler &operator=(const CgiHandler &obj);
};

#endif