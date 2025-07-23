#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "common.hpp"

#define MAX_CHUNK_SIZE 100000000
#define MAX_URI_LENGTH 2048

struct UploadedFile {
    std::string filename;
    std::string content;
};


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
        std::string getQueryString(void) const;
    	const std::map<std::string, std::vector<UploadedFile> >& getUploadedFiles() const;

        // Other functions
        bool    parse(const std::string &raw); 
        bool validateBody(std::size_t maxBodySize);
        void    printMembers(void) const;
        const std::map<std::string, std::string>& getFormData() const;

    private:
        std::string _method;
        std::string _target;
        std::string _version;
        std::map<std::string, std::string> _headers;
        std::string _body;
        int _parseErrorCode;
        std::map<std::string, std::string> _formData;
        std::map<std::string, std::vector<UploadedFile> > _uploadedFiles;

        bool checkMethod(const std::string &method);
        bool checkTarget(const std::string &target);
        bool isValidPercentEncoding(const std::string& uri);
        bool checkVersion(const std::string &version);
        bool parseRequestLine(std::istream &stream);
        bool parseHeaders(std::istream &stream);
        bool isSupportedMethod(const std::string &method);
        bool handleChunkedEncoding(void);
        bool checkRequiredLengthHeader(void);
        bool checkContentLength(void);
        bool checkUploadedFileSizes(std::size_t maxBodySize);
        bool checkTotalBodySize(std::size_t maxBodySize);
        bool decodeChunkedBody(void);
        void parseBody(void);
        bool parseMultipartFormData(const std::string &boundary);
        bool extractContentDisposition(const std::string &headers, std::string &contentDisposition) const;
        bool extractFieldNames(const std::string &contentDisposition, std::string &name, std::string &filename) const;
        bool extractPartContent(std::string::size_type &prevPos, std::string::size_type &nextBoundaryPos,
            const std::string &boundary, std::string &partContent) const;
};

#endif
