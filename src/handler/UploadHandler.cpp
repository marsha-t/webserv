#include "../../includes/UploadHandler.hpp"

UploadHandler::UploadHandler(const Route &route, const ServerConfig &config) : _route(route), _config(config) {}

UploadHandler::~UploadHandler(void) {}

void UploadHandler::handle(const Request &req, Response &res)
{
	if (req.getMethod() != "POST")
	{
		res.setError(405, _config);
		return;
	}

	std::string uploadDir = _route.getUploadDir();
	if (uploadDir.empty())
	{
		res.setError(500, _config); // Upload directory not configured
		return;
	}
	struct stat s;
	if (stat(uploadDir.c_str(), &s) != 0 || !S_ISDIR(s.st_mode) || access(uploadDir.c_str(), W_OK) != 0)
	{
		res.setError(500, _config); // path doesn't exist, isn't directory or isn't writable
		return;
	}
	const std::map<std::string, std::vector<UploadedFile> >& uploadedFiles = req.getUploadedFiles();
	if (uploadedFiles.empty())
	{
		res.setError(400, _config); // No files uploaded
		return;
	}
	for (std::map<std::string, std::vector<UploadedFile> >::const_iterator it = uploadedFiles.begin(); it != uploadedFiles.end(); ++it)
	{
		const std::vector<UploadedFile>& fileList = it->second;
		for (std::vector<UploadedFile>::const_iterator fileIt = fileList.begin(); fileIt != fileList.end(); ++fileIt)
		{
			std::string safeFilename = sanitizeFilename(fileIt->filename);
			std::string fullPath = uploadDir + "/" + safeFilename;
			if (access(fullPath.c_str(), F_OK) == 0) {
				res.setError(409, _config); // Conflict: file exists
				return;
			}
			if (!isSafePath(fullPath, uploadDir)) {
				res.setError(403, _config);
				return;
			}
			if (!saveFile(fullPath, fileIt->content))
			{
				res.setError(500, _config);
				return;
			}
		}
	}
	res.setStatusLine(201, httpStatusMessage(201));
	res.setHeader("Location", uploadDir);
}

// To avoid filenames like ../../../etc/passwd that will escape intended upload directory
// keeps alphanumerics, dots, underscores and hyphens; replace all others with _
std::string UploadHandler::sanitizeFilename(const std::string& filename)
{
	std::string clean;
	for (size_t i = 0; i < filename.size(); ++i)
	{
		char c = filename[i];
		if (std::isalnum(c) || c == '.' || c == '_' || c == '-')
			clean += c;
		else
			clean += '_';
	}
	if (clean.empty())
		clean = "upload";
	return clean;
}

bool UploadHandler::saveFile(const std::string &filename, const std::string &content) const
{
	std::ofstream ofs(filename.c_str(), std::ios::binary);
	if (!ofs.is_open())
		return false;
	ofs.write(content.c_str(), content.length());
	ofs.close();
	return true;
}

bool UploadHandler::isSafePath(const std::string &path, const std::string &baseDir) const
{
	char *resolvedPath = realpath(path.c_str(), NULL);
	char *resolvedBase = realpath(baseDir.c_str(), NULL);

	if (!resolvedPath || !resolvedBase)
	{
		free(resolvedPath);
		free(resolvedBase);
		return false;
	}

	std::string pathStr(resolvedPath);
	std::string baseStr(resolvedBase);

	free(resolvedPath);
	free(resolvedBase);

	if (pathStr == baseStr)
		return true;
	if (pathStr.compare(0, baseStr.length(), baseStr) == 0 &&
		(pathStr[baseStr.length()] == '/' || pathStr.length() == baseStr.length()))
		return true;

	return false;
}
