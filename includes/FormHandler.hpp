#ifndef FORMHANDLER_HPP
#define FORMHANDLER_HPP

#include "common.hpp"
#include "IRequestHandler.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Route.hpp"
#include "ServerConfig.hpp"

class FormHandler : public IRequestHandler {
    
    public:
        // Constructor
        FormHandler(void);
    	FormHandler(const FormHandler& other);
        FormHandler(const Route& route, const ServerConfig& config);
        
        // Operator
    	FormHandler& operator=(const FormHandler& other);

        // Destructor
        virtual ~FormHandler(void);

        virtual void handle(const Request& req, Response& res);

    private:
        Route _route;
        ServerConfig _config;
};

#endif