#include "../../includes/utils.hpp"

std::string trimR(const std::string &line)
{
	if (!line.empty() && line[line.size() - 1] == '\r')
        return line.substr(0, line.size() - 1);
    return line;
}

std::string toString(int n)
{
	std::stringstream ss;
	ss << n;
	return ss.str();
}

std::string toLower(const std::string &str)
{
	std::string lowerStr = str;
	for (size_t i = 0; i < lowerStr.length(); ++i)
		lowerStr[i] = std::tolower(lowerStr[i]);
	return lowerStr;
}

