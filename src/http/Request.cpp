#include "../../includes/Request.hpp"

Request::Request(void)
    : _method(), _target(), _version(), _headers(),
      _body(), _parseErrorCode(0), _formData(), _uploadedFiles() {}

Request::Request(const Request &obj): _method(obj._method), _target(obj._target), \
		_version(obj._version), _headers(obj._headers), _body(obj._body), \
		_parseErrorCode(obj._parseErrorCode), _formData(obj._formData), _uploadedFiles(obj._uploadedFiles) {}
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
		_formData = obj._formData;
		_uploadedFiles = obj._uploadedFiles;
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

std::string Request::getQueryString(void) const
{
	std::string::size_type queryPos = _target.find('?');
	if (queryPos != std::string::npos)
		return _target.substr(queryPos + 1);
	return "";
}

bool	Request::parse(const std::string &raw)
{
	std::string::size_type headerEnd = raw.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		return false;
	std::string headers = raw.substr(0, headerEnd);
	_body = raw.substr(headerEnd + 4);
	std::istringstream stream(headers);

	if (!parseRequestLine(stream))
	{
		if (_parseErrorCode == 0)
			_parseErrorCode = 400;
		return false;
	}
	if (!parseHeaders(stream))
	{
		if (_parseErrorCode == 0)
			_parseErrorCode = 400;
		return false;
	}

	return true;
}

bool Request::parseRequestLine(std::istream &stream)
{
	std::string line;
	if (!std::getline(stream, line))
	{
		_parseErrorCode = 400;
		return false;
	}

	line = trimR(line);
	std::istringstream requestLine(line);
	if (!(requestLine >> _method >> _target >> _version))
	{
		_parseErrorCode = 400;
		return false;
	}

	return (checkMethod(_method) && checkVersion(_version) && checkTarget(_target));
}

bool Request::checkMethod(const std::string &method)
{
	if (method != "GET" && method != "POST" && method != "DELETE")
	{
		_parseErrorCode = 405;
		return false;
	}
	return true;
}

bool Request::checkTarget(const std::string &target)
{
	if (target.empty() || target[0] != '/')
	{
		_parseErrorCode = 400;
		return false;
	}
	if (target.size() > MAX_URI_LENGTH)
	{
		_parseErrorCode = 414;
		return false;
	}
	return true;
}

bool Request::checkVersion(const std::string &version)
{
	if (version == "HTTP/1.1" || version == "HTTP/1.0")
		return true;
	_parseErrorCode = 426;
	return false;
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
	if (!isSupportedMethod(_method))
	{
		_parseErrorCode = 501;
		return false;
	}

	if (!handleChunkedEncoding())
		return false;

	if (!checkRequiredLengthHeader())
		return false;

	if (!checkContentLength())
		return false;

	if (!checkUploadedFileSizes(maxBodySize))
		return false;

	if (!checkTotalBodySize(maxBodySize))
		return false;

	parseBody();
	return true;
}

bool Request::isSupportedMethod(const std::string &method)
{
	const std::string supportedMethods[] = { "GET", "POST", "DELETE" };
	for (size_t i = 0; i < 3; ++i)
	{
		if (method == supportedMethods[i])
			return true;
	}
	return false;
}

bool Request::handleChunkedEncoding(void)
{
	std::string transferEncoding = ::toLower(getHeader("transfer-encoding"));
	if (transferEncoding == "chunked")
	{
		if (!decodeChunkedBody())
		{
			_parseErrorCode = 400;
			return false;
		}
	}
	return true;
}

bool Request::checkRequiredLengthHeader(void)
{
	if (_method == "POST")
	{
		std::string cl = getHeader("content-length");
		std::string te = ::toLower(getHeader("transfer-encoding"));
		if (cl.empty() && te != "chunked")
		{
			_parseErrorCode = 411;
			return false;
		}
	}
	return true;
}

bool Request::checkContentLength(void)
{
	std::string contentLength = getHeader("content-length");

	if (!contentLength.empty())
	{
		char *endptr;
		long length = std::strtol(contentLength.c_str(), &endptr, 10);
		if (*endptr != '\0' || length < 0)
		{
			std::cerr << "Invalid Content-Length header: " << contentLength << std::endl;
			_parseErrorCode = 400;
			return false;
		}

		std::size_t bodySize = _body.size();
		if (static_cast<std::size_t>(length) > bodySize)
		{
			std::cerr << "Incomplete body: expected " << length << " bytes, got " << bodySize << std::endl;
			_parseErrorCode = 400;
			return false;
		}
		if (bodySize > static_cast<std::size_t>(length))
			_body = _body.substr(0, length);
	}
	return true;
}

bool Request::checkUploadedFileSizes(std::size_t maxBodySize)
{
	std::string contentType = getHeader("content-type");

	if (contentType.find("multipart/form-data") != std::string::npos)
	{
		for (std::map<std::string, std::vector<UploadedFile> >::const_iterator it = _uploadedFiles.begin();
		     it != _uploadedFiles.end(); ++it)
		{
			const std::vector<UploadedFile>& fileList = it->second;
			for (std::vector<UploadedFile>::const_iterator fit = fileList.begin();
			     fit != fileList.end(); ++fit)
			{
				if (fit->content.size() > maxBodySize)
				{
					_parseErrorCode = 413;
					return false;
				}
			}
		}
	}
	return true;
}

bool Request::checkTotalBodySize(std::size_t maxBodySize)
{
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
			break;
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

const std::map<std::string, std::string>& Request::getFormData() const { return _formData; }

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

    std::cout << "\n--- Form Data ---" << std::endl;
    if (_formData.empty())
        std::cout << "(no form data)" << std::endl;
    else
    {
        for (std::map<std::string, std::string>::const_iterator it = _formData.begin(); it != _formData.end(); ++it) {
            std::cout << it->first << ": " << it->second << std::endl;
        }
    }
	std::cout << "\n--- Uploaded Files ---" << std::endl;
	if (_uploadedFiles.empty())
		std::cout << "(no uploaded files)" << std::endl;
	else
	{
		for (std::map<std::string, std::vector<UploadedFile> >::const_iterator it = _uploadedFiles.begin(); it != _uploadedFiles.end(); ++it)
		{
			std::cout << "Field: " << it->first << std::endl;
			for (std::vector<UploadedFile>::const_iterator fit = it->second.begin(); fit != it->second.end(); ++fit)
			{
				std::cout << "  Filename: " << fit->filename << ", Size: " << fit->content.size() << " bytes" << std::endl;
			}
		}
	}

    std::cout << "=======================" << std::endl;
}

const std::map<std::string, std::vector<UploadedFile> >& Request::getUploadedFiles() const 
{
	return _uploadedFiles;
}

void Request::parseBody()
{
    std::string contentType = getHeader("content-type");
    if (contentType.find("application/x-www-form-urlencoded") != std::string::npos)
    {
        std::istringstream iss(_body);
        std::string pair;
        while (std::getline(iss, pair, '&'))
        {
            std::string::size_type eqPos = pair.find('=');
            if (eqPos != std::string::npos)
            {
                std::string key = pair.substr(0, eqPos);
                std::string value = pair.substr(eqPos + 1);
                _formData[key] = value;
            }
        }
    }
    else if (contentType.find("multipart/form-data") != std::string::npos)
    {
        std::size_t boundaryPos = contentType.find("boundary=");
		if (boundaryPos == std::string::npos || boundaryPos + 9 >= contentType.length()) {
			_parseErrorCode = 400;
			return;
		}
		std::string boundary = "--" + contentType.substr(boundaryPos + 9);
        if (!parseMultipartFormData(boundary))
			_parseErrorCode = 400;
    }
	else if (!contentType.empty())
	{
		_parseErrorCode = 415;
	}
}

bool Request::parseMultipartFormData(const std::string &boundary)
{
    std::string::size_type pos = 0;
    std::string::size_type prevPos = 0;

    // Skip preamble
    pos = _body.find(boundary, prevPos);
    if (pos == std::string::npos)
        return false;
    prevPos = pos + boundary.length() + 2;

    while (true)
    {
        // Find part headers
        pos = _body.find("\r\n\r\n", prevPos);
        if (pos == std::string::npos)
            return false;
        std::string partHeaders = _body.substr(prevPos, pos - prevPos);
        prevPos = pos + 4;

        std::string contentDisposition;
        if (!extractContentDisposition(partHeaders, contentDisposition))
            return false;

        std::string name, filename;
        if (!extractFieldNames(contentDisposition, name, filename))
            return false;

		std::string partContent;
        std::string::size_type nextBoundaryPos;
        if (!extractPartContent(prevPos, nextBoundaryPos, boundary, partContent))
            return false;

        if (!filename.empty())
		{
			UploadedFile file;
			file.filename = filename;
			file.content = partContent;
            _uploadedFiles[name].push_back(file);
		}
        else if (!name.empty())
            _formData[name] = partContent;
        else
            return false;

        bool isFinal = (_body.substr(nextBoundaryPos, boundary.length() + 4) == boundary + "--\r\n");
        prevPos = nextBoundaryPos + boundary.length() + (isFinal ? 4 : 2);

        if (isFinal)
            break;
    }

    return true;
}

bool Request::extractContentDisposition(const std::string &headers, std::string &contentDisposition) const
{
    std::istringstream stream(headers);
    std::string line;
    while (std::getline(stream, line))
    {
        if (line.find("Content-Disposition:") != std::string::npos)
        {
            contentDisposition = line;
            return true;
        }
    }
    return false;
}

bool Request::extractFieldNames(const std::string &contentDisposition, std::string &name, std::string &filename) const
{
    std::string::size_type namePos = contentDisposition.find("name=\"");
    if (namePos != std::string::npos)
    {
        namePos += 6;
        std::string::size_type nameEnd = contentDisposition.find("\"", namePos);
        if (nameEnd == std::string::npos)
            return false;
        name = contentDisposition.substr(namePos, nameEnd - namePos);
		name = trimR(name);
    }

    std::string::size_type filenamePos = contentDisposition.find("filename=\"");
    if (filenamePos != std::string::npos)
    {
        filenamePos += 10;
        std::string::size_type filenameEnd = contentDisposition.find("\"", filenamePos);
        if (filenameEnd == std::string::npos)
            return false;
        filename = contentDisposition.substr(filenamePos, filenameEnd - filenamePos);
		filename = trimR(filename);
    }

    return true;
}

bool Request::extractPartContent(std::string::size_type &prevPos, std::string::size_type &nextBoundaryPos, const std::string &boundary, std::string &partContent) const
{
    nextBoundaryPos = _body.find(boundary, prevPos);
    if (nextBoundaryPos == std::string::npos || nextBoundaryPos < 2 || _body.substr(nextBoundaryPos - 2, 2) != "\r\n")
        return false;

    partContent = _body.substr(prevPos, nextBoundaryPos - prevPos - 2); // exclude \r\n
    return true;
}
