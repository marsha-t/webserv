#include "../../includes/Route.hpp"

Route::Route(void): _isRedirect(false), _hasClientMaxBodySize(false) {}
Route::Route(const Route &obj): _location(obj._location), _root(obj._root), _methods(obj._methods), \
 _indexFiles(obj._indexFiles), _autoindex(obj._autoindex), _uploadDir(obj._uploadDir), \
 _isRedirect(obj._isRedirect), _redirectStatusCode(obj._redirectStatusCode), _redirectURL(obj._redirectURL), _cgi(obj._cgi), \
 _clientMaxBodySize(obj._clientMaxBodySize), _hasClientMaxBodySize(obj._hasClientMaxBodySize) {}
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
		_clientMaxBodySize = obj._clientMaxBodySize;
		_hasClientMaxBodySize = obj._hasClientMaxBodySize;
	}
	return (*this);
}

void	Route::setLocation(const std::string &location) { _location = location; }
void	Route::setRoot(const std::string &root) 
{ 
	_root = root;
	if (!_root.empty() && _root[_root.size() - 1] == '/')
		_root = _root.substr(0, _root.size() - 1);

}
void	Route::addAllowedMethod(const std::string &method) 
{ 
	if (method == "GET" || method == "POST" || method == "DELETE")
		_methods.push_back(method);
	else
		throw std::runtime_error("Invalid method in configuration");
}
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
void	Route::setClientMaxBodySize(std::size_t clientMaxBodySize) 
{ 
	_clientMaxBodySize = clientMaxBodySize; 
	_hasClientMaxBodySize = true;
}

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
std::size_t Route::getClientMaxBodySize(void) const { return _clientMaxBodySize; }
bool Route::hasClientMaxBodySize(void) const { return _hasClientMaxBodySize; }

bool Route::isMethodAllowed(const std::string &method) const {
	const std::vector<std::string> &allowed = getMethods();
	if (allowed.empty())
		return (method == "GET");
	else
		return std::find(allowed.begin(), allowed.end(), method) != allowed.end();
}
