#ifndef UTILS_HPP
#define UTILS_HPP

#include "common.hpp"

template <typename T>
std::string toString(T n)
{
	std::ostringstream oss;
	oss << n;
	return oss.str();
}

std::string toLower(const std::string &s);

#endif
