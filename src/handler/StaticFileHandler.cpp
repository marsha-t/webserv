#include "../../includes/StaticFileHandler.hpp"

StaticFileHandler::StaticFileHandler(const StaticFileHandler &obj): _route(obj._route), _config(obj._config) {}
StaticFileHandler::StaticFileHandler(const Route &route, const ServerConfig &config): _route(route), _config(config) {}
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
		res.setError(405, "Method Not Allowed", _config);
		return ;
	}
	std::string path = _route.getRoot() + req.getTarget().substr(_route.getLocation().length());

	if (!isSafePath(path))
	{
		res.setError(403, "Forbidden", _config);
		return ;
	}
	struct stat s;
	if (stat(path.c_str(), &s) != 0) // File or directory does not exist
	{
		res.setError(404, "Not Found", _config);
		return;
	}

	if (S_ISDIR(s.st_mode)) 
	{
		const std::vector<std::string> &indexes = _route.getIndexFiles();
		bool found = false;
		for (size_t i = 0; i < indexes.size(); ++i)
		{
			std::string candidate = path + "/" + indexes[i];
			struct stat index_s;
			if (stat(candidate.c_str(), &index_s) == 0 && S_ISREG(index_s.st_mode) && access(candidate.c_str(), R_OK) == 0)
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
				std::string listing = generateDirectoryListing(path, req.getTarget());
				res.setStatusLine(200, "OK");
				res.setHeader("Content-Type", "text/html");
				res.setBody(listing);
				return;
			}
			else
			{
				res.setError(403, "Forbidden", _config);
				return;
			}
		}
	}

	if (req.getMethod() == "DELETE")
	{
		if (remove(path.c_str()) != 0)
		{
			res.setError(500, "Internal Server Error", _config);
			return;
		}
		res.setStatusLine(204, "No Content");
		return;
	}

	// For GET/POST (non-DELETE) requests, serve the file
	// Check read permissions
	if (access(path.c_str(), R_OK) != 0)
	{
		res.setError(403, "Forbidden", _config);
		return;
	}

	try 
	{
		std::string content;
		if (!readFile(path, content))
		{
			res.setError(500, "Internal Server Error", _config);
			return;
		}
		std::string mimeType = getMimeType(path);
		res.setFile(content, mimeType);
	}
	catch (std::exception &e)
	{
		res.setError(500, "Internal Server Error", _config);
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

bool StaticFileHandler::readFile(const std::string &path, std::string &content) const
{
	std::ifstream file(path.c_str());
	if (!file.is_open())
		return false;
	std::ostringstream buffer;
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

#include <dirent.h>

std::string StaticFileHandler::generateDirectoryListing(const std::string &dirPath, const std::string &uriPath) const
{
    std::string html = "<!DOCTYPE html>\n";
    html += "<html>\n<head><title>Index of " + uriPath + "</title></head>\n";
    html += "<body><h1>Index of " + uriPath + "</h1><hr><pre>\n";

    DIR *dir = opendir(dirPath.c_str());
    if (!dir)
    {
        return "Error: Could not open directory";
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;
        if (name == ".")
            continue;

        std::string fullPath = dirPath + "/" + name;
        std::string displayPath = uriPath;
        if (displayPath[displayPath.length() - 1] != '/')
            displayPath += '/';
        displayPath += name;

        struct stat s;
        if (stat(fullPath.c_str(), &s) == 0 && S_ISDIR(s.st_mode))
        {
            displayPath += '/';
        }
        html += "<a href=\"" + displayPath + "\">" + name + "</a>\n";
    }
    closedir(dir);

    html += "</pre><hr></body>\n</html>\n";
    return html;
}