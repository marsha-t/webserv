#ifndef UTILS_HPP
#define UTILS_HPP

template <typename T>
std::string toString(T n)
{
	std::ostringstream oss;
	oss << n;
	return oss.str();
}

#endif
