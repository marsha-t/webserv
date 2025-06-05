#ifndef ROUTE_HPP
#define ROUTE_HPP

#include "common.hpp"

class Route
{
	public:
		// Constructor
		Route(void);
		Route(const Route &obj);
	
		// Destructor
		~Route(void);

		// Operator
		Route &operator=(const Route &obj);

		// Setters
		void	setLocation(const std::string &location);
		void	setRoot(const std::string &root);
		void	addMethod(const std::string &method);
		void	addIndexFile(const std::string &index);
		void	setAutoindex(bool autoindex);
		void	setUploadDir(const std::string &dir);
		void	setRedirect(int statusCode, const std::string &url);
		void	addCGI(const std::string &ext, const std::string &exec);
		// ...

		// Getters
		const std::string &getLocation(void) const;
		const std::string &getRoot(void) const;
		const std::vector<std::string> &getMethods(void) const;
		const std::vector<std::string> &getIndexFiles(void) const;
		bool getAutoindex(void) const;
		const std::string &getUploadDir(void) const;
		bool isRedirect(void) const;
		int getRedirectStatusCode(void) const;
		const std::string &getRedirectURL(void) const;
		const std::map<std::string, std::string> &getCGI(void) const;
		// ...

	private:
		std::string _location;
		std::string _root;
		std::vector<std::string> _methods;
		std::vector<std::string> _indexFiles;
		bool _autoindex;
		std::string _uploadDir;
		bool _isRedirect;
		int _redirectStatusCode;
		std::string _redirectURL;
		std::map<std::string, std::string> _cgi;
		// ...
};

#endif