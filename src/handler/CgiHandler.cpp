#include "../../includes/CgiHandler.hpp"
#include "../../includes/utils.hpp"





CgiHandler::CgiHandler(const Route &route, const ServerConfig &config) : _route(route), _config(config) {}

CgiHandler::~CgiHandler(void) {}

void CgiHandler::handle(const Request &req, Response &res)
{
	std::string scriptPath = _route.getRoot() + req.getTarget();
	std::string cgiOutput = executeCgi(req, res, scriptPath);

	res.setStatusLine(200, "OK");
	res.setBody(cgiOutput);
}

std::map<std::string, std::string> CgiHandler::getCgiEnv(const Request &req) const
{
	std::map<std::string, std::string> env;

	env["AUTH_TYPE"] = "";
	env["CONTENT_LENGTH"] = toString(req.getBody().length());
	env["CONTENT_TYPE"] = req.getHeader("Content-Type");
	env["GATEWAY_INTERFACE"] = "CGI/1.1";
	env["PATH_INFO"] = req.getTarget();
	env["PATH_TRANSLATED"] = _route.getRoot() + req.getTarget();
	env["QUERY_STRING"] = req.getQueryString();
	env["REMOTE_ADDR"] = ""; // Should be set to the client's IP address
	env["REMOTE_IDENT"] = "";
	env["REMOTE_USER"] = "";
	env["REQUEST_METHOD"] = req.getMethod();
	env["REQUEST_URI"] = req.getTarget();
	env["SCRIPT_NAME"] = req.getTarget();
	env["SERVER_NAME"] = _config.getServerNames()[0];
	env["SERVER_PORT"] = toString(_config.getPort());
	env["SERVER_PROTOCOL"] = "HTTP/1.1";
	env["SERVER_SOFTWARE"] = "webserv/1.0";
	env["REDIRECT_STATUS"] = "200"; // Required for PHP-CGI

	return env;
}

std::string CgiHandler::executeCgi(const Request &req, Response &res, const std::string &scriptPath) const
{
	std::map<std::string, std::string> envMap = getCgiEnv(req);
	char **env = new char *[envMap.size() + 1];
	int i = 0;
	for (std::map<std::string, std::string>::const_iterator it = envMap.begin(); it != envMap.end(); ++it)
	{
		std::string envString = it->first + "=" + it->second;
		env[i] = new char[envString.length() + 1];
		strcpy(env[i], envString.c_str());
		i++;
	}
	env[i] = NULL;

	int pipe_in[2];
	int pipe_out[2];
	if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1)
	{
		throw std::runtime_error("pipe failed");
	}

	pid_t pid = fork();
	if (pid == -1)
	{
		res.setError(500, "Internal Server Error", _config);
		throw std::runtime_error("fork failed");
	}

	if (pid == 0)
	{
		close(pipe_in[1]);
		close(pipe_out[0]);
		dup2(pipe_in[0], STDIN_FILENO);
		dup2(pipe_out[1], STDOUT_FILENO);
		close(pipe_in[0]);
		close(pipe_out[1]);

		if (chdir(_route.getRoot().c_str()) != 0)
		{
			res.setError(500, "Internal Server Error", _config);
			throw std::runtime_error("chdir failed");
		}

		std::string ext = req.getTarget().substr(req.getTarget().find_last_of("."));
		const char *cgiPath = _route.getCGI().at(ext).c_str();
		char *const argv[] = {const_cast<char *>(cgiPath), const_cast<char *>(scriptPath.c_str()), NULL};

		execve(cgiPath, argv, env);
		res.setError(500, "Internal Server Error", _config);
		throw std::runtime_error("execve failed");
	}
	else
	{
		close(pipe_in[0]);
		close(pipe_out[1]);

		if (req.getHeader("transfer-encoding") == "chunked")
		{
			// Handle chunked request body
			std::string body = req.getBody();
			std::istringstream stream(body);
			while (true)
			{
				std::string line;
				if (!std::getline(stream, line))
					break;
				line = trimR(line);
				if (line.empty())
					continue;
				std::string::size_type semicolon = line.find(';');
				if (semicolon != std::string::npos)
					line = line.substr(0, semicolon);
				std::istringstream size(line);
				std::size_t chunkSize;
				size >> std::hex >> chunkSize;
				if (size.fail() || !size.eof())
					break;
				if (chunkSize == 0)
					break;
				std::string chunk(chunkSize, '\0');
				stream.read(&chunk[0], chunkSize);
				if (stream.gcount() != static_cast<std::streamsize>(chunkSize))
					break;
				write(pipe_in[1], chunk.c_str(), chunk.size());
				char end[2];
				stream.read(end, 2);
				if (end[0] != '\r' || end[1] != '\n')
					break;
			}
		}
		else
		{
			// Handle non-chunked request body
			write(pipe_in[1], req.getBody().c_str(), req.getBody().length());
		}

		close(pipe_in[1]);

		std::string cgiOutput;
		char buffer[4096];
		ssize_t bytesRead;
		while ((bytesRead = read(pipe_out[0], buffer, sizeof(buffer))) > 0)
		{
			cgiOutput.append(buffer, bytesRead);
		}

		close(pipe_out[0]);

		int status;
		waitpid(pid, &status, 0);

		for (int j = 0; j < i; ++j)
		{
			delete[] env[j];
		}
		delete[] env;

		return cgiOutput;
	}
}

