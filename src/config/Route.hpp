#ifndef ROUTE_HPP
#define ROUTE_HPP

#include "../../includes/common.hpp"

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
		void	setRedirect(const std::string &url);
		void	addCGIExtension(const std::string &ext, const std::string &exec);
		// ...

		// Getters
		const std::string &getLocation(void) const;
		const std::string &getRoot(void) const;
		const std::vector<std::string> &getMethods(void) const;
		const std::vector<std::string> &getIndexFiles(void) const;
		bool getAutoindex(void) const;
		const std::string &getUploadDir(void) const;
		const std::string &getRedirect(void) const;
		const std::map<std::string, std::string> &getCGIExtensions(void) const;
		// ...

	private:
		std::string _location;
		std::string _root;
		std::vector<std::string> _methods;
		std::vector<std::string> _indexFiles;
		bool _autoindex;
		std::string _uploadDir;
		std::string _redirect;
		std::map<std::string, std::string> _cgiExtensions;
		// ...
};

#endif