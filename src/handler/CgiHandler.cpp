#include "../../includes/CgiHandler.hpp"

CgiHandler::CgiHandler(const Route &route, const ServerConfig &config) : _route(route), _config(config) {}

CgiHandler::~CgiHandler(void) {}

void CgiHandler::handle(const Request &req, Response &res)
{
	std::string scriptPath = _route.getRoot() + req.getTarget();
	std::string cgiOutput = executeCgi(req, res, scriptPath);
	parseCgiResponse(cgiOutput, res);
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
	char **env = createEnvArray(envMap);

	int pipe_in[2], pipe_out[2];
	if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1)
		throw std::runtime_error("pipe failed");

	pid_t pid = fork();
	if (pid == -1)
	{
		res.setError(500, _config);
		throw std::runtime_error("fork failed");
	}

	if (pid == 0)
		handleChildProcess(req, scriptPath, env, pipe_in, pipe_out);
	else
	{
		close(pipe_in[0]);
		close(pipe_out[1]);

		writeRequestBody(req, pipe_in[1]);
		std::string cgiOutput = readCgiOutput(pipe_out[0]);

		int status;
		waitpid(pid, &status, 0);

		// Cleanup
		for (int j = 0; env[j]; ++j)
			delete[] env[j];
		delete[] env;

		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		{
			res.setError(500, _config);
			return "";
		}
		return cgiOutput;
	}
	return "";
}

char **CgiHandler::createEnvArray(const std::map<std::string, std::string>& envMap) const
{
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
	return env;
}

void CgiHandler::handleChildProcess(const Request &req, const std::string &scriptPath, char **env, int pipe_in[2], int pipe_out[2]) const
{
	close(pipe_in[1]);
	close(pipe_out[0]);
	dup2(pipe_in[0], STDIN_FILENO);
	dup2(pipe_out[1], STDOUT_FILENO);
	close(pipe_in[0]);
	close(pipe_out[1]);

	if (chdir(_route.getRoot().c_str()) != 0)
	{
		perror("chdir failed");
		_exit(1);
	}

	std::string ext = req.getTarget().substr(req.getTarget().find_last_of("."));
	std::map<std::string, std::string>::const_iterator it = _route.getCGI().find(ext);
	if (it == _route.getCGI().end())
		_exit(1); // Extension not supported

	const char *cgiPath = it->second.c_str();
	char *const argv[] = {const_cast<char *>(cgiPath), const_cast<char *>(scriptPath.c_str()), NULL};
	execve(cgiPath, argv, env);
	perror("execve failed");
	_exit(1);
}

void CgiHandler::writeRequestBody(const Request &req, int writeFd) const
{
	if (req.getHeader("transfer-encoding") == "chunked")
	{
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

			std::istringstream sizeStream(line);
			std::size_t chunkSize;
			sizeStream >> std::hex >> chunkSize;

			if (sizeStream.fail() || !sizeStream.eof())
				break;
			if (chunkSize == 0)
				break;

			std::string chunk(chunkSize, '\0');
			stream.read(&chunk[0], chunkSize);
			if (stream.gcount() != static_cast<std::streamsize>(chunkSize))
				break;

			if (::write(writeFd, chunk.c_str(), chunk.size()) == -1)
				break;

			char end[2];
			stream.read(end, 2);
			if (stream.gcount() != 2 || end[0] != '\r' || end[1] != '\n')
				break;
		}
	}
	else
	{
		if (::write(writeFd, req.getBody().c_str(), req.getBody().length()) == -1)
		{
			// optionally log or handle write failure
		}
	}
	close(writeFd);
}

std::string CgiHandler::readCgiOutput(int readFd) const
{
	std::string output;
	char buffer[4096];
	ssize_t bytesRead;
	while ((bytesRead = read(readFd, buffer, sizeof(buffer))) > 0)
	{
		output.append(buffer, bytesRead);
	}
	close(readFd);
	return output;
}

void CgiHandler::parseCgiResponse(const std::string &cgiOutput, Response &res) const
{
	std::string::size_type headerEnd = cgiOutput.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
	{
		// Malformed CGI response — treat entire output as body
		res.setStatusLine(200, httpStatusMessage(200));
		res.setBody(cgiOutput);
		return;
	}

	std::string headerBlock = cgiOutput.substr(0, headerEnd);
	std::string body = cgiOutput.substr(headerEnd + 4);

	std::istringstream headerStream(headerBlock);
	std::string line;
	int statusCode = 200;

	while (std::getline(headerStream, line))
	{
		// Trim \r at end (since getline stops at \n)
		if (!line.empty() && line[line.length() - 1] == '\r')
			line.erase(line.length() - 1);

		std::string::size_type colon = line.find(':');
		if (colon == std::string::npos)
		{
			if (line.substr(0, 7) == "Status ")
			{
				std::istringstream iss(line.substr(7));
				iss >> statusCode;
			}
			continue;
		}

		std::string key = line.substr(0, colon);
		std::string value = line.substr(colon + 1);
		// Left trim
		value.erase(0, value.find_first_not_of(" \t"));

		if (key == "Status")
		{
			std::istringstream iss(value);
			iss >> statusCode;
		}
		else
			res.setHeader(key, value);
	}

	res.setStatusLine(statusCode, httpStatusMessage(statusCode));
	res.setBody(body);
}
