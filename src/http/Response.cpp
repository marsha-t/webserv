#include "Response.hpp"

Response::Response(void): _httpVersion("HTTP/1.1") {}

Response::Response(const Response &obj): _httpVersion(obj._httpVersion), _statusLine(obj._statusLine), _headers(obj._headers), _body(obj._body) {}

Response::~Response(void) {}

Response &Response::operator=(const Response &obj) 
{
	if (this != &obj)
	{
		_httpVersion = obj._httpVersion;
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
	_statusLine = line.str();
}

void    Response::setHeader(const std::string &key, const std::string &value)
{
	_headers[key] = value;
}

void Response::setBody(const std::string &body)
{
	_body = body;
}

std::string Response::toString(void) const
{
	std::ostringstream oss;
	oss << _statusLine << "\r\n";
	for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
	{
		oss << it->first << ": " << it->second << "\r\n";
	}

	// Empty line after headers
	oss << "\r\n";

	// Body
	oss << _body;

	return oss.str();
}