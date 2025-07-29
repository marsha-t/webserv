#include "../../includes/CgiHandler.hpp"

CgiHandler::CgiHandler(const Route &route, const ServerConfig &config) : _route(route), _config(config) {}

CgiHandler::~CgiHandler(void) {}

void CgiHandler::handle(const Request &req, Response &res)
{
	std::string rawTarget = req.getTarget();
	std::string::size_type qpos = rawTarget.find('?');
	std::string cleanedTarget = (qpos != std::string::npos) ? rawTarget.substr(0, qpos) : rawTarget;
	std::string relativePath = _route.getRoot() + cleanedTarget;

	char *absPath = realpath(relativePath.c_str(), NULL);

	if (!absPath) {
		debugMsg("realpath failed for script: " + relativePath);
		res.setError(404, _config); 
		return;
	}

	std::string scriptPath(absPath);
	free(absPath);

	std::string cgiOutput = executeCgi(req, res, scriptPath);
	if (!res.isError())
		parseCgiResponse(cgiOutput, res);
}

std::map<std::string, std::string> CgiHandler::getCgiEnv(const Request &req) const
{
	std::map<std::string, std::string> env;

	env["AUTH_TYPE"] = "";
	env["CONTENT_LENGTH"] = toString(req.getBody().length());
	env["CONTENT_TYPE"] = req.getHeader("content-type");
	env["GATEWAY_INTERFACE"] = "CGI/1.1";
	env["PATH_INFO"] = req.getTarget();
	env["PATH_TRANSLATED"] = _route.getRoot() + req.getTarget();
	env["QUERY_STRING"] = req.getQueryString();
	env["REMOTE_ADDR"] = "";
	env["REMOTE_IDENT"] = "";
	env["REMOTE_USER"] = "";
	env["REQUEST_METHOD"] = req.getMethod();
	env["REQUEST_URI"] = req.getTarget();
	env["SCRIPT_NAME"] = req.getTarget();
	env["SERVER_NAME"] = _config.getServerNames()[0];
	env["SERVER_PORT"] = toString(_config.getPort());
	env["SERVER_PROTOCOL"] = "HTTP/1.1";
	env["SERVER_SOFTWARE"] = "webserv/1.0";
	env["REDIRECT_STATUS"] = "200";

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

		if (!writeRequestBody(req, pipe_in[1]))
		{
			waitpid(pid, NULL, 0);
			res.setError(500, _config);
			return "";
		}

		std::string cgiOutput = readCgiOutput(pipe_out[0]);
		int status;
		waitpid(pid, &status, 0);
		int exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;

		// Cleanup
		for (int j = 0; env[j]; ++j)
			delete[] env[j];
		delete[] env;
		
		if (exitCode == 126) {
			debugMsg("CGI exited with 126 — permission denied");
			res.setError(403, _config);
			return "";
		}

		if (exitCode != 0 || cgiOutput.empty()) {
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
	std::string target = req.getTarget();
	std::size_t queryPos = target.find('?');
	if (queryPos != std::string::npos)
		target = target.substr(0, queryPos);

	std::size_t dotPos = target.find_last_of(".");
	std::string ext = (dotPos != std::string::npos) ? target.substr(dotPos) : "";

	std::map<std::string, std::string>::const_iterator it = _route.getCGI().find(ext);

	if (it == _route.getCGI().end())
	{
		_exit(1); // Extension not supported
	}

	const char *cgiPath = it->second.c_str();
	char *const argv[] = {const_cast<char *>(cgiPath), const_cast<char *>(scriptPath.c_str()), NULL};
	if (access(scriptPath.c_str(), X_OK) != 0)
	{
		perror("Script is not executable");
		_exit(126);
	}
	execve(cgiPath, argv, env);
	perror("execve failed");
	_exit(1);
}

bool CgiHandler::writeRequestBody(const Request &req, int writeFd) const
{
	const std::string& body = req.getBody();
	if (body.empty()) {
		close(writeFd);
		return true;
	}

	ssize_t bytesWritten = write(writeFd, body.c_str(), body.length());

	bool success = true;
	if (bytesWritten == -1)
	{
		perror("write to CGI stdin failed");
		success = false;
	}
	else if (bytesWritten == 0)
	{
		std::cerr << "[error] write() returned 0 — pipe closed?" << std::endl;
		success = false;
	}
	else if (static_cast<size_t>(bytesWritten) < body.length())
	{
		std::cerr << "[warning] Partial write to CGI stdin: "
		          << bytesWritten << "/" << body.length() << " bytes written" << std::endl;
		success = false;
	}

	close(writeFd);
	return success;
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
	if (bytesRead == -1)
	{
		perror("read from CGI stdout failed");
		output.clear();
	}
	close(readFd);
	return output;
}


void CgiHandler::parseCgiResponse(const std::string &cgiOutput, Response &res) const
{
	std::string::size_type headerEnd = cgiOutput.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		headerEnd = cgiOutput.find("\n\n");

	if (headerEnd == std::string::npos)
	{
		debugMsg("Malformed CGI response: missing header-body separator");
		res.setError(500, _config);
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
