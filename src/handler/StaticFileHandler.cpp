#include "../../includes/StaticFileHandler.hpp"


static std::map<std::string, std::string> createMimeTypeMap()
{
	std::map<std::string, std::string> m;
	m["html"] = "text/html";
	m["htm"] = "text/html";
	m["css"] = "text/css";
	m["js"] = "application/javascript";
	m["png"] = "image/png";
	m["jpg"] = "image/jpeg";
	m["jpeg"] = "image/jpeg";
	m["gif"] = "image/gif";
	m["txt"] = "text/plain";
	m["json"] = "application/json";
	m["pdf"] = "application/pdf";
	return m;
}

static const std::map<std::string, std::string> mimeTypeMap = createMimeTypeMap();

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
	std::string path = _route.getRoot() + req.getTarget().substr(_route.getLocation().length());
	if (!isSafePath(path)) 
	{
		res.setError(403, _config);
		return;
	}

	struct stat s;
	if (stat(path.c_str(), &s) != 0) 
	{
		res.setError(404, _config);
		return;
	}

	if (req.getMethod() == "DELETE") 
	{
		if (S_ISDIR(s.st_mode)) 
		{
			res.setError(403, _config);
			return;
		}
		if (!handleDelete(res, path))
			res.setError(500, _config);
		return;
	}
	if (S_ISDIR(s.st_mode)) 
	{
		if (!handleDirectory(req, res, path))
			return;
	}
	if (!serveFile(res, path)) {
		res.setError(500, _config);
	}
}

std::string StaticFileHandler::resolvePath(const Request &req) const {
	return _route.getRoot() + req.getTarget().substr(_route.getLocation().length());
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

	free(realPath);
	free(rootPath);

	if (realPathStr == rootPathStr)
		return true;
	if (realPathStr.compare(0, rootPathStr.length(), rootPathStr) == 0 && realPathStr[rootPathStr.length()] == '/')
		return true;

	return false;
}

bool StaticFileHandler::handleDirectory(const Request &req, Response &res, std::string &path) const {
	std::string indexPath;
	if (!findIndexFile(path, indexPath)) {
		if (_route.getAutoindex()) {
			std::string listing = generateDirectoryListing(path, req.getTarget());
			res.setStatusLine(200, httpStatusMessage(200));
			res.setHeader("Content-Type", "text/html");
			res.setBody(listing);
		} else {
			res.setError(403, _config);
		}
		return false;
	}
	path = indexPath;
	return true;
}

bool StaticFileHandler::handleDelete(Response &res, const std::string &path) const {
	if (remove(path.c_str()) != 0)
		return false;
	res.setStatusLine(204, httpStatusMessage(204));
	return true;
}

bool StaticFileHandler::serveFile(Response &res, const std::string &path) const {
	if (access(path.c_str(), R_OK) != 0)
		return false;
	std::string content;
	if (!readFile(path, content))
		return false;
	std::string mimeType = getMimeType(path);
	res.setFile(content, mimeType);
	return true;
}

bool StaticFileHandler::findIndexFile(const std::string &dir, std::string &indexPath) const
{
	const std::vector<std::string> &indexes = _route.getIndexFiles();
	for (size_t i = 0; i < indexes.size(); ++i)
	{
		std::string candidate = dir + "/" + indexes[i];
		struct stat s;
		if (stat(candidate.c_str(), &s) == 0 && S_ISREG(s.st_mode) && access(candidate.c_str(), R_OK) == 0)
		{
			indexPath = candidate;
			return true;
		}
	}
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
	for (size_t i = 0; i < ext.length(); ++i)
		ext[i] = std::tolower(ext[i]);

	std::map<std::string, std::string>::const_iterator it = mimeTypeMap.find(ext);
	if (it != mimeTypeMap.end())
		return it->second;
	return "application/octet-stream";
}

std::string StaticFileHandler::generateDirectoryListing(const std::string &dirPath, const std::string &uriPath) const
{
	std::string safeUriPath = uriPath;
	if (safeUriPath.empty() || safeUriPath[safeUriPath.length() - 1] != '/')
		safeUriPath += '/';

	std::string html = "<!DOCTYPE html>\n";
	html += "<html>\n<head><title>Index of " + safeUriPath + "</title></head>\n";
	html += "<body><h1>Index of " + safeUriPath + "</h1><hr><pre>\n";

	DIR *dir = opendir(dirPath.c_str());
	if (!dir)
		return "Error: Could not open directory";

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL)
	{
		std::string name = entry->d_name;

		// Skip hidden files and directories: ".", "..", ".env", ".git", etc.
		if (name == "." || (name[0] == '.' && name != ".."))
			continue;

		std::string fullPath = dirPath + "/" + name;
		std::string displayPath = safeUriPath + name;

		struct stat s;
		if (stat(fullPath.c_str(), &s) == 0 && S_ISDIR(s.st_mode))
		{
			displayPath += '/';
			name += '/';
		}
		html += "<a href=\"" + displayPath + "\">" + name + "</a>\n";
	}
	closedir(dir);

	html += "</pre><hr></body>\n</html>\n";
	return html;
}
