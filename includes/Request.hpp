#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "common.hpp"

#define MAX_CHUNK_SIZE 100000000

class Request
{
    public:
        // Constructor
        Request(void);
        Request(const Request &obj);
        
        // Destructor
        ~Request(void);
        
        // Operators
        Request &operator=(const Request &obj);
        
        // Getters
        const std::string &getMethod() const;
        const std::string &getTarget() const;
        const std::string &getVersion() const;
        const std::map<std::string, std::string>& getHeaders() const;
        std::string getHeader(const std::string &key) const;
        const std::string &getBody() const;
        int getParseErrorCode(void) const;

        // Other functions
        bool    parse(const std::string &raw); 
        bool validateBody(std::size_t maxBodySize);
        void    printMembers(void) const;
        
    private:
        std::string _method;
        std::string _target;
        std::string _version;
        std::map<std::string, std::string> _headers;
        std::string _body;
        int _parseErrorCode;

        std::string trimR(const std::string &line);
        bool checkMethod(const std::string &method);
        bool checkTarget(const std::string &target);
        bool checkVersion(const std::string &version);
        bool parseRequestLine(std::istream &stream);
        bool parseHeaders(std::istream &stream);
        bool decodeChunkedBody(void);


};

#endif
