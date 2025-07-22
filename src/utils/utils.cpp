// #include "../../includes/utils.hpp"
#include "../../includes/common.hpp"

std::string trimR(const std::string &line)
{
	if (!line.empty() && line[line.size() - 1] == '\r')
        return line.substr(0, line.size() - 1);
    return line;
}

std::string toString(int n)
{
	std::stringstream ss;
	ss << n;
	return ss.str();
}

std::string toLower(const std::string &str)
{
	std::string lowerStr = str;
	for (size_t i = 0; i < lowerStr.length(); ++i)
		lowerStr[i] = std::tolower(lowerStr[i]);
	return lowerStr;
}

void debugMsg(const std::string &msg)
{
	std::cerr << BLUE << "[DEBUG] " << msg << RESET << std::endl;
}

void debugMsg(const std::string &msg, int fd)
{
	std::cerr << BLUE << "[DEBUG] " << msg << fd << RESET << std::endl;
}

void errorMsg(const std::string &msg)
{
	std::cerr << RED << "[ERROR] " << msg << RESET << std::endl;
}

void errorMsg(const std::string &msg, int fd)
{
	std::cerr << RED << "[ERROR] FD " << fd << ": " << msg << RESET << std::endl;
}

void fatalError(const std::string &context)
{
	perror(context.c_str());
	throw std::runtime_error(context);
}

std::string httpStatusMessage(int code) {
	switch (code) {
		case 200: return "OK";
		case 201: return "Created";
		case 204: return "No Content";
		case 301: return "Moved Permanently";
		case 302: return "Found";
		case 303: return "See Other";
		case 307: return "Temporary Redirect";
		case 308: return "Permanent Redirect";
		case 400: return "Bad Request";
		case 401: return "Unauthorized";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 409: return "Conflict";
		case 411: return "Length Required";
		case 413: return "Payload Too Large";
		case 414: return "URI Too Long";
		case 415: return "Unsupported Media Type";
		case 426: return "Upgrade Required";
		case 500: return "Internal Server Error";
		case 501: return "Not Implemented";
		case 502: return "Bad Gateway";
		case 503: return "Service Unavailable";
		default:  return "Bad Request";
	}
}

std::string joinPath(const std::string &base, const std::string &relative) {
	if (base.empty())
		return relative;
	if (relative.empty())
		return base;

	if (base[base.size() - 1] == '/' && relative[0] == '/')
		return base + relative.substr(1); // remove one slash
	if (base[base.size() - 1] != '/' && relative[0] != '/')
		return base + "/" + relative; // add missing slash
	return base + relative; // just concatenate
}
