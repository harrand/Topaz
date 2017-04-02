#ifndef UTILITY_HPP
#define UTILITY_HPP
#include <string>
#include <cstdio>
#include <cmath>
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include "vector.hpp"

namespace StringUtility
{
	std::string toLower(std::string data);
	std::string toUpper(std::string data);
	bool beginsWith(const std::string& what, const std::string& withwhat);
	bool endsWith(const std::string& what, const std::string& withwhat);
	bool contains(const std::string& what, char withwhat);
	std::vector<std::string> splitString(const std::string& split, char delim);
	std::string replaceAllChar(const std::string& str, char toreplace, const std::string& replacewith);
	std::string substring(const std::string& str, unsigned int begin, unsigned int end);
}

namespace CastUtility
{
	// Implementation must be kept inside the header to avoid linker errors.
	template <typename T>
	std::string toString(T obj)
	{
		std::ostringstream oss;
		oss << obj;
		return oss.str();
	}
	
	template <typename T>
	T fromString(const std::string& s)
	{
		T ret;
		std::istringstream ss(s);
		ss >> ret;
		return ret;
	}
}

class Force
{
public:
	Force(Vector3F size = Vector3F());
	Vector3F getSize() const;
	void setSize(Vector3F size);
	Force operator+(const Force& other) const;
	Force operator-(const Force& other) const;
	Force operator*(float rhs) const;
	Force operator/(float rhs) const;
	void operator+=(const Force& other);
	void operator-=(const Force& other);
private:
	Vector3F size;
};

#endif //UTILITY_HPP