#include "../../includes/UploadHandler.hpp"

UploadHandler::UploadHandler(const Route &route, const ServerConfig &config) : _route(route), _config(config) {}

UploadHandler::~UploadHandler(void) {}

void UploadHandler::handle(const Request &req, Response &res)
{
	if (req.getMethod() != "POST")
	{
		res.setError(405, "Method Not Allowed", _config);
		return;
	}

	std::string uploadDir = _route.getUploadDir();
	if (uploadDir.empty())
	{
		res.setError(500, "Internal Server Error", _config); // Upload directory not configured
		return;
	}

	const std::map<std::string, std::string>& uploadedFiles = req.getUploadedFiles();
	if (uploadedFiles.empty())
	{
		res.setError(400, "Bad Request", _config); // No files uploaded
		return;
	}

	for (std::map<std::string, std::string>::const_iterator it = uploadedFiles.begin(); it != uploadedFiles.end(); ++it)
	{
		std::string filename = uploadDir + "/" + it->first; // Use original filename
		if (!saveFile(filename, it->second))
		{
			res.setError(500, "Internal Server Error", _config);
			return;
		}
	}

	res.setStatusLine(201, "Created");
	res.setHeader("Location", uploadDir);
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
