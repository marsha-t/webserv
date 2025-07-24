#include "../../includes/ServerConfig.hpp"
#include "../../includes/Response.hpp"

static std::string normalizeHeaderKey(const std::string &key)
{
	std::string result;
	bool capitalizeNext = true;
	for (std::string::const_iterator it = key.begin(); it != key.end(); ++it)
	{
		if (*it == '-')
		{
			result += *it;
			capitalizeNext = true;
		}
		else if (capitalizeNext)
		{
			result += std::toupper(*it);
			capitalizeNext = false;
		}
		else
			result += std::tolower(*it);
	}
	return result;
}

Response::Response(void): _httpVersion("HTTP/1.1"), _statusCode(200) {}

Response::Response(const Response &obj): _httpVersion(obj._httpVersion), _statusCode(obj._statusCode), _statusLine(obj._statusLine), _headers(obj._headers), _body(obj._body) {}

Response::~Response(void) {}

Response &Response::operator=(const Response &obj) 
{
	if (this != &obj)
	{
		_httpVersion = obj._httpVersion;
		_statusCode = obj._statusCode;
		_statusLine = obj._statusLine;
		_headers = obj._headers;
		_body = obj._body;
	}
	return (*this);
}

void    Response::setStatusLine(int code, const std::string &message)
{
	std::ostringstream	line;
	line << _httpVersion << " " << code << " " << message;
	_statusCode = code;
	_statusLine = line.str();
}

void    Response::setHeader(const std::string &key, const std::string &value)
{
	_headers[normalizeHeaderKey(key)] = value;
}

void Response::setBody(const std::string &body)
{
	_body = body;
	std::istringstream iss(_statusLine);
	std::string version;
	int code;
	iss >> version >> code;
	if (code != 204 && code != 304 && (code < 100 || code >= 200))
		setHeader("Content-Length", ::toString(_body.size()));
}

void Response::setError(int code, const ServerConfig &config)
{
	std::string message = httpStatusMessage(code);
	setStatusLine(code, message);
	setHeader("Content-Type", "text/html");

	std::map<int, std::string>::const_iterator it = config.getErrorPages().find(code);
	if (it != config.getErrorPages().end())
	{
		std::string errorPagePath = it->second;
		std::ifstream file(errorPagePath.c_str());
		if (file.is_open())
		{
			std::stringstream buffer;
			buffer << file.rdbuf();
			std::string content = buffer.str();
			file.close();
			if (!content.empty())
				setBody(content);
			else
			{
				setDefaultErrorBody(code, message);
			}
		}
		else
		{
			setDefaultErrorBody(code, message);
		}
	}
	else
	{
		setDefaultErrorBody(code, message);
	}
}

void Response::setDefaultErrorBody(int code, const std::string &message)
{
	setBody("<html><head><title>" + ::toString(code) + " " + message + "</title></head>"
			"<body><center><h1>" + ::toString(code) + " " + message + "</h1></center>"
			"<hr><center>webserv</center></body></html>");
}

void Response::setFile(const std::string &body, const std::string &mimeType)
{
	setStatusLine(200, httpStatusMessage(200));
	setHeader("Content-Type", mimeType);
	setBody(body);
}

// Code 204 and 304 don't include message body and no content-length header
std::string Response::toString(void) const
{
	std::ostringstream oss;
	oss << _statusLine << "\r\n";

	// Extract status code
	std::istringstream iss(_statusLine);
	std::string httpVersion;
	int code = 0;
	iss >> httpVersion >> code;

	// Suppress Content-Length and body for 204 and 304
	if (code == 204 || code == 304)
	{
		// Remove Content-Length and any related headers
		for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
		{
			if (it->first != "Content-Length")
				oss << it->first << ": " << it->second << "\r\n";
		}
		oss << "\r\n"; // End of headers
	}
	else
	{
		for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
		{
			oss << it->first << ": " << it->second << "\r\n";
		}
		oss << "\r\n" << _body;
	}
	return oss.str();
}

bool Response::isError() const
{
	return _statusCode >= 400;
}
