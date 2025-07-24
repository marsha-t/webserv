#include "../../includes/FormHandler.hpp"
FormHandler::FormHandler(void) {}

FormHandler::FormHandler(const FormHandler& other): _route(other._route), _config(other._config) {}

FormHandler::FormHandler(const Route& route, const ServerConfig& config): _route(route), _config(config) {}

FormHandler& FormHandler::operator=(const FormHandler& other)
{
	if (this != &other)
	{
		_route = other._route;
		_config = other._config;
	}
	return *this;
}

FormHandler::~FormHandler(void) {}

void FormHandler::handle(const Request& req, Response& res)
{
	const std::map<std::string, std::string>& formData = req.getFormData();

	if (!req.getBody().empty() && formData.empty())
	{
		res.setError(400, _config);
		return;
	}

	std::ostringstream html;
	html << "<html><body><h2>Form submission</h2>";

	if (formData.empty())
		html << "<p>No form fields received.</p>";
	else
	{
		html << "<ul>";
		for (std::map<std::string, std::string>::const_iterator it = formData.begin(); it != formData.end(); ++it)
			html << "<li>" << it->first << " = " << it->second << "</li>";
		html << "</ul>";
	}

	html << "</body></html>";

	res.setStatusLine(200, "OK");
	res.setHeader("Content-Type", "text/html");
	res.setBody(html.str());
}
