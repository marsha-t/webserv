#include "Route.hpp"

Route::Route(void) {}
Route::Route(const Route &obj): _location(obj._location), _root(obj._root), _methods(obj._methods), \
 _indexFiles(obj._indexFiles), _autoindex(obj._autoindex), _uploadDir(obj._uploadDir), \
 _isRedirect(obj._isRedirect), _redirectStatusCode(obj._redirectStatusCode), _redirectURL(obj._redirectURL), _cgi(obj._cgi) {}
Route::~Route(void) {}
Route &Route::operator=(const Route &obj) 
{
	if (this != &obj)
	{
		_location = obj._location;
		_root = obj._root;
		_methods = obj._methods;
		_indexFiles = obj._indexFiles;
		_autoindex = obj._autoindex;
		_uploadDir = obj._uploadDir;
		_isRedirect = obj._isRedirect;
		_redirectStatusCode = obj._redirectStatusCode;
		_redirectURL = obj._redirectURL;
		_cgi = obj._cgi;
	}
	return (*this);
}

void	Route::setLocation(const std::string &location) { _location = location; }
void	Route::setRoot(const std::string &root) { _root = root; }
void	Route::addMethod(const std::string &method) { _methods.push_back(method); }
void	Route::addIndexFile(const std::string &index) { _indexFiles.push_back(index); }
void	Route::setAutoindex(bool autoindex) { _autoindex = autoindex; }
void	Route::setUploadDir(const std::string &dir) { _uploadDir = dir; }
void	Route::setRedirect(int statusCode, const std::string &url)
{
	_isRedirect = true;
	_redirectStatusCode = statusCode;
	_redirectURL = url;
}


void	Route::addCGI(const std::string &ext, const std::string &exec) { _cgi[ext] = exec; }

const std::string &Route::getLocation(void) const { return _location; }
const std::string &Route::getRoot(void) const { return _root; }
const std::vector<std::string> &Route::getMethods(void) const { return _methods; }
const std::vector<std::string> &Route::getIndexFiles(void) const { return _indexFiles; }
bool Route::getAutoindex(void) const { return _autoindex; }
const std::string &Route::getUploadDir(void) const { return _uploadDir; }
bool Route::isRedirect(void) const { return _isRedirect; }
int Route::getRedirectStatusCode(void) const { return _redirectStatusCode; }
const std::string &Route::getRedirectURL(void) const { return _redirectURL; }
const std::map<std::string, std::string> &Route::getCGI(void) const { return _cgi; }
