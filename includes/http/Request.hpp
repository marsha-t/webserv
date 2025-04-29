#ifndef REQUEST_HPP
#define REQUEST_HPP

class Request
{
    private:
        std::string _method;
        std::string _target;
        std::string _version;
        std::map<std::string, std::string> _headers;
        std::string _body;
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
            const std::string &getHeader(const std::string &key) const;
            const std::string &getBody() const;

        // Other functions
        bool parse(const std::string &raw); 
};

#endif
