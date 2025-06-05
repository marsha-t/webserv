#include "../../includes/StaticFileHandler.hpp"

StaticFileHandler::StaticFileHandler(void) {}
StaticFileHandler::StaticFileHandler(const StaticFileHandler &obj): _route(obj._route) {}
StaticFileHandler::StaticFileHandler(const Route &route): _route(route) {}
StaticFileHandler &StaticFileHandler::operator=(const StaticFileHandler &obj)
{
	if (this != &obj)
		_route = obj._route;
	return *this;
}
StaticFileHandler::~StaticFileHandler(void) {}

/*
	- Check that method is allowed
	- Create full path and check that it is safe
	- If it is a directory, look for index file
		- if found, serve it
		- if no index file, check if autoindex
			- if autoindex OFF, return 403 Forbidden
			- if autoindex ON, return HTML page of directory listing
	- if file doesn't exist, 404 Not Found
	- else read file and general 200 response
*/
void StaticFileHandler::handle(const Request &req, Response &res)
{
	const std::vector<std::string> &allowed = _route.getMethods();
	if (std::find(allowed.begin(), allowed.end(), req.getMethod()) == allowed.end())
	{
		res.setError(405, "Method Not Allowed");
		return ;
	}
	std::string path = _route.getRoot() + "/" + req.getTarget();

	if (!isSafePath(path))
	{
		res.setError(403, "Forbidden");
		return ;
	}
	struct stat s;
	if (stat(path.c_str(), &s) == 0 && S_ISDIR(s.st_mode)) 
	{
		const std::vector<std::string> &indexes = _route.getIndexFiles();
		bool found = false;
		for (size_t i = 0; i < indexes.size(); ++i)
		{
			std::string candidate = path + "/" + indexes[i];
			if (fileExists(candidate, req.getMethod()))
			{
				path = candidate;
				found = true;
				break;
			}
		}
		if (!found)
		{
			if (_route.getAutoindex())
			{
				// TODO: implement autoindex
				res.setStatusLine(200, "OK");
				res.setHeader("Content-Type", "text/plain");
				res.setBody("Autoindex placeholder");
				return;
			}
			else
			{
				res.setStatusLine(403, "Forbidden");
				res.setHeader("Content-Type", "text/html");
				res.setBody("<html><head><title>403 Forbidden</title></head>"
							"<body><center><h1>403 Forbidden</h1></center>"
							"<hr><center>webserv</center></body></html>");
				return;
			}
		}
	}
	if (!fileExists(path, req.getMethod()))
	{
		res.setError(404, "Not Found");
		return;
	}
	try 
	{
		std::string content;
		if (!readFile(path, content))
		{
			res.setError(404, "Not Found");
		}
		std::string mimeType = getMimeType(path);
		res.setFile(content, mimeType);
	}
	catch (std::exception &e)
	{
		res.setError(500, "Internal Server Error");
	}
}

// Checks whether target path is within root directory
bool StaticFileHandler::isSafePath(const std::string &path) const
{
	char *realPath = realpath(path.c_str(), NULL);
	char *rootPath = realpath(_route.getRoot().c_str(), NULL);
	
	if (!realPath || !rootPath)
	{
		free(realPath);
		free(rootPath);
		return false;
	}
	
	std::string rootPathStr(rootPath);
	std::string realPathStr(realPath);

	if (realPathStr.compare(0, rootPathStr.length(), rootPathStr) == 0
		&& (realPathStr[rootPathStr.length()] == '/' || realPathStr.length() == rootPathStr.length()))
	{
		free(realPath);
		free(rootPath);
		return true;
	}
	free(realPath);
	free(rootPath);
	return false;
}

bool StaticFileHandler::fileExists(const std::string &path, const std::string &method) const
{
	if (access(path.c_str(), F_OK) != 0)
		return false;
	if (method == "GET" && access(path.c_str(), R_OK) != 0)
		return false;
	return true;
}
bool StaticFileHandler::readFile(const std::string &path, std::string &content) const
{
	std::ifstream file(path.c_str());
	if (!file.is_open())
		return false;
	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();
	content = buffer.str();
	return true;
}

std::string StaticFileHandler::getMimeType(const std::string &filename) const
{
	std::string::size_type dot = filename.rfind('.');
	if (dot == std::string::npos)
		return "application/octet-stream";
	std::string ext = filename.substr(dot + 1);
	if (ext == "html" || ext == "htm")
		return "text/html";
	else if (ext == "css")
		return "text/css";
	else if (ext == "js")
		return "application/javascript";
	else if (ext == "png")
		return "image/png";
	else if (ext == "jpg" || ext == "jpeg")
		return "image/jpeg";
	else if (ext == "gif")
		return "image/gif";
	else if (ext == "txt")
		return "text/plain";
	else if (ext == "json")
		return "application/json";
	else if (ext == "pdf")
		return "application/pdf";
	else
		return "application/octet-stream";
}