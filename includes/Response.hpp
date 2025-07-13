#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "common.hpp"
#include "utils.hpp"
#include "ServerConfig.hpp"
class Response
{
	private:
		std::string _httpVersion;
		std::string _statusLine;
		std::map<std::string, std::string> _headers;
		std::string _body;
	public:
		// Constructor
		Response(void);
		Response(const Response &obj);

		// Destructor
		~Response(void);

		// Operators
		Response &operator=(const Response &obj);

		// Setters
		void setStatusLine(int code, const std::string &message);
		void setHeader(const std::string &key, const std::string &value);
		void setBody(const std::string &body);
		void setError(int code, const std::string &message, const ServerConfig &config);
		void setFile(const std::string &body, const std::string &mimeType);
	
		// Other functions
		std::string toString(void) const;
};

#endif
