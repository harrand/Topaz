#ifndef UTILITY_HPP
#define UTILITY_HPP
#include <string>
#include <cstdio>
#include <cmath>
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>

namespace StringUtility
{
	std::string toString(float obj);
	inline void toLower(std::string& data);
	inline void toUpper(std::string& data);
	bool beginsWith(std::string what, std::string withwhat);
	bool endsWith(std::string what, std::string withwhat);
	bool contains(std::string what, char withwhat);
	std::vector<std::string> splitString(std::string split, char delim);
	std::string replaceAllChar(std::string str, char toreplace, std::string replacewith);
	std::string substring(std::string str, unsigned int begin, unsigned int end);
}

namespace CastUtility
{
	template <typename T>
	T fromString(std::string s)
	{
		T ret;
		std::istringstream ss(s);
		ss >> ret;
		return ret;
	}
}

#endif //UTILITY_HPP