#include "../../includes/Request.hpp"

Request::Request(void) {}
Request::Request(const Request &obj): _method(obj._method), _target(obj._target), _version(obj._version), _headers(obj._headers), _body(obj._body), _parseErrorCode(obj._parseErrorCode) {}
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
		_parseErrorCode = obj._parseErrorCode;
	}
	return (*this);
}

const std::string &Request::getMethod() const { return _method; }
const std::string &Request::getTarget() const { return _target; }
const std::string &Request::getVersion() const { return _version; }
const std::map<std::string, std::string>&Request::getHeaders() const { return _headers; }
std::string Request::getHeader(const std::string &key) const
{
	std::map<std::string, std::string>::const_iterator it = _headers.find(key);
	if (it != _headers.end())
		return it->second;
	return "";
}

const std::string &Request::getBody() const { return _body; }
int Request::getParseErrorCode(void) const { return _parseErrorCode; }

bool	Request::parse(const std::string &raw)
{
	std::string::size_type headerEnd = raw.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		return false;
	std::string headers = raw.substr(0, headerEnd);
	_body = raw.substr(headerEnd + 4);
	std::istringstream stream(headers);

	if (!parseRequestLine(stream))
		return false;
	if (!parseHeaders(stream))
		return false;
	return true;
}

bool Request::parseRequestLine(std::istream &stream)
{
	std::string line;
	if (!std::getline(stream, line))
		return false;

	line = trimR(line);
	std::istringstream requestLine(line);
	if (!(requestLine >> _method >> _target >> _version))
		return false;

	return (checkMethod(_method) && checkVersion(_version) && checkTarget(_target));
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

bool Request::parseHeaders(std::istream &stream)
{
	std::string line;
	while (std::getline(stream, line))
	{
		line = trimR(line);
		std::string::size_type colon = line.find(':');
		if (colon == std::string::npos)
		{
			_parseErrorCode = 400;
			return false;
		}

		std::string key = line.substr(0, colon);
		std::string value = line.substr(colon + 1);

		size_t i = 0;
		while (i < value.size() && std::isspace(value[i]))
			++i;

		value = value.substr(i);
		_headers[::toLower(key)] = value;
	}
	return true;
}

bool Request::validateBody(std::size_t maxBodySize)
{
	std::string contentLength = getHeader("content-length");
	std::string transferEncoding = ::toLower(getHeader("transfer-encoding"));

	if (transferEncoding == "chunked")
	{
		return decodeChunkedBody();
	}
	if (!contentLength.empty())
	{
		char *endptr;
		long length = std::strtol(contentLength.c_str(), &endptr, 10);
		if (*endptr != '\0' || length < 0)
		{
			std::cerr << "Invalid Content-Length header: " << contentLength << std::endl;
            return false; // TODO should this be an exception?
		}
		if (static_cast<size_t>(length) > _body.size())
		{
			std::cerr << "Incomplete body: expected " << length << " bytes, got " << _body.size() << std::endl;
            return false;  // TODO should this be an exception?
		}
		if (_body.size() > static_cast<size_t>(length))
			_body = _body.substr(0, length);
	}
	if (_body.size() > maxBodySize)
	{
		_parseErrorCode = 413;
		return false;
	}
	return true;
}

// Checks: 
//	- Valid hex parsing (returns false otherwise)
// 	- Check no extra characters after hex
// 	- Enforce reasonable MAX_CHUNK_SIZE
// 	- Body ends with chunk terminator ()
// 	- Next line is same size as chunk size
// 	- Line ends with \r\n
//  - Reject chunk extensions ()
//  - Empty lines (just skip)
bool Request::decodeChunkedBody()
{
	std::string decoded;
	std::istringstream stream(_body);

	while (true)
	{
		std::string line;
		if (!std::getline(stream, line))
			return false;

		line = trimR(line);
		if (line.empty())
			continue;
		
		std::string::size_type semicolon = line.find(';');
		if (semicolon != std::string::npos)
			line = line.substr(0, semicolon);

		std::istringstream size(line);
		std::size_t chunkSize;
		size >> std::hex >> chunkSize;
		if (size.fail() || !size.eof())
			return false;
		if (chunkSize > MAX_CHUNK_SIZE)
			return false;
		
		if (chunkSize == 0)
		{
			std::string trailer;
			if (!std::getline(stream, trailer) || trimR(trailer) != "")
    			return false;
			break;
		}
		std::string chunk(chunkSize, '\0');
		stream.read(&chunk[0], chunkSize);
		if (stream.gcount() != static_cast<std::streamsize>(chunkSize))
			return false;
		decoded += chunk;

		char end[2];
		stream.read(end, 2);
		if (end[0] != '\r' || end[1] != '\n')
			return false;
	}
	_body = decoded;
	return true;
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
