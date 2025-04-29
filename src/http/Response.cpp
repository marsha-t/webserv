#include "Response.hpp"

Response::Response(void) {}

Response::Response(const Response &obj) {}

Response::~Response(void) {}

Response &Response::operator=(const Response &obj) {}

void    Response::setStatusLine(int code, const std::string &message)
{
	std::ostringstream	line;
	line << "HTTP/1.1" << code << " " << message << "\r\n";
	_statusLine = line.str();
}

void    Response::setHeader(const std::string &key, const std::string &value)
{

}

void setBody(const std::string &body)
{

}

std::string Response::toString(void) const
{

}