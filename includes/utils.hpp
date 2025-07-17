#ifndef UTILS_HPP
#define UTILS_HPP

#include "common.hpp"

std::string trimR(const std::string &line);
std::string toString(int n);
std::string toLower(const std::string &str);
void    debugMsg(const std::string &msg);
void	debugMsg(const std::string &msg, int fd);
void    errorMsg(const std::string &msg);
void	errorMsg(const std::string &msg, int fd);
void	fatalError(const std::string &context);
std::string httpStatusMessage(int code);

#endif
