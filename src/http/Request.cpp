#include "Request.hpp"

Request::Request(void) {}
Request::Request(const Request &obj): _method(obj._method), _target(obj._target), _version(obj._version), _headers(obj._headers), _body(obj._body) {}
Request::~Request(void) {}
Request &Request::operator=(const Request &obj) 
{
	if (this != &obj)
	{
		_method = obj._method;
	 	_target = obj._target;
		_version = obj._version;
		_headers = obj._headers;
		_body = obj._body;
	}
	return (*this);
}

const std::string &Request::getMethod() const { return _method; }
const std::string &Request::getTarget() const { return _target; }
const std::string &Request::getVersion() const { return _version; }
const std::map<std::string, std::string>&Request::getHeaders() const { return _headers; }
const std::string &Request::getBody() const { return _body; }

bool	Request::parse(const std::string &raw)
{
	size_t	headerEnd = raw.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		return false;
	std::string headers = raw.substr(0, headerEnd);
	_body = raw.substr(headerEnd + 4);
	std::istringstream stream(headers);

	// Parse request line
	std::string line;
	if (!std::getline(stream, line))
		return false;
	line = trimR(line);
	std::istringstream requestLine(line);
	if (!(requestLine >> _method >> _target >> _version))
		return false;
	if (!checkMethod(_method) || !checkVersion(_version) || !checkTarget(_target))
	    return false;
	
	// Parse remaining headers
	while (std::getline(stream, line))
	{
		line = trimR(line);
		size_t colon = line.find(':');
		if (colon == std::string::npos)
			return false;
		std::string key = line.substr(0, colon);
		std::string value = line.substr(colon + 1);

		size_t i = 0; 
		while (i < value.size() && std::isspace(value[i]))
			++i;
		value = value.substr(i);
		_headers[key] = value;
	}
	return (true);
}

void    Request::printMembers(void) const
{
	std::cout << "=== Request Members ===" << std::endl;
    std::cout << "Method:  " << _method << std::endl;
    std::cout << "Target:  " << _target << std::endl;
    std::cout << "Version: " << _version << std::endl;
    std::cout << "\n--- Headers ---" << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }

    std::cout << "\n--- Body ---" << std::endl;
    if (_body.empty())
        std::cout << "(no body)" << std::endl;
    else
        std::cout << _body << std::endl;

    std::cout << "=======================" << std::endl;
}

std::string Request::trimR(const std::string &line)
{
	if (!line.empty() && line[line.size() - 1] == '\r')
        return line.substr(0, line.size() - 1);
    return line;
}

bool Request::checkMethod(const std::string &method)
{
	if (method != "GET" && method != "POST" && method != "DELETE")
		return false;
	return true;
}

bool Request::checkTarget(const std::string &target)
{
	if (target.empty() || target[0] != '/')
		return false;
	return true;
}

bool Request::checkVersion(const std::string &version)
{
	return (version == "HTTP/1.1" || version == "HTTP/1.0");
}
