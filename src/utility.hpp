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

namespace CastUtility
{
	// Implementation must be kept inside the header to avoid linker errors.
	template <typename T>
	inline std::string toString(T obj)
	{
		std::ostringstream oss;
		oss << obj;
		return oss.str();
	}
	
	template <typename T>
	inline T fromString(const std::string& s)
	{
		T ret;
		std::istringstream ss(s);
		ss >> ret;
		return ret;
	}
}

namespace StringUtility
{
	inline std::string toLower(std::string data)
	{
		std::transform(data.begin(), data.end(), data.begin(), ::tolower);
		return data;
	}

	inline std::string toUpper(std::string data)
	{
		std::transform(data.begin(), data.end(), data.begin(), ::toupper);
		return data;
	}

	inline bool beginsWith(const std::string& what, const std::string& withwhat)
	{
		unsigned int strsize = what.length(), withsize = withwhat.length();
		const char* str = what.c_str();
		const char* with = withwhat.c_str();
		if(strsize < withsize || str[0] != with[0])
			return false;
		for(unsigned int iter = 0; iter < withsize; iter++)
		{
			char str_at = str[iter], with_at = with[iter];
			if(str_at == with_at)
			{
				if(iter == withsize)
					return true;
			}
			else
			{
				return false;
			}
		}
	}

	inline bool endsWith(const std::string& what, const std::string& withwhat)
	{
		unsigned int strsize = what.length(), withsize = withwhat.length();
		const char* str = what.c_str();
		const char* with = withwhat.c_str();
		if(strsize < withsize || str[strsize-withsize] != with[0])
			return false;
		for(unsigned int iter = 0; iter < withsize; iter++)
		{
			char str_at = str[strsize-(withsize-iter)], with_at = with[iter];
			if(str_at == with_at)
			{
				if(iter == withsize)
					return true;
			}
			else
			{
				return false;
			}
		}
	}

	inline bool contains(const std::string& what, char withwhat)
	{
		const char* whatcstr = what.c_str();
		for(unsigned int i = 0; i < what.length(); i++)
			if(whatcstr[i] == withwhat)
				return true;
	}

	inline std::vector<std::string> splitString(const std::string& split, char delim)
	{
		unsigned int strsize = split.length();
		const char* str = split.c_str();
		std::vector<std::string> ret;
		unsigned int numcount = 0;
		std::unordered_map<unsigned int, unsigned int> num_pos;
		for(unsigned int i = 0; i < strsize; i++)
		{
			char cur = str[i];
			if(cur == delim)
			{
				num_pos[numcount] = i;
				numcount++;
			}
			if(i == (strsize-1))
			{
				for(unsigned int j = 0; j < numcount; j++)
				{
					unsigned int prevpos;
					if(j == 0)
						prevpos = 0;
					unsigned int posdelim = num_pos[j];
					if(prevpos == 0)
					{
						std::string sub = split.substr(prevpos, (posdelim-prevpos));
						ret.push_back(sub);
					}
					else
					{
						std::string sub = split.substr((prevpos+1), (posdelim-prevpos-1));
						ret.push_back(sub);
					}
					if(j == (numcount-1))
					{
						prevpos+=(posdelim-prevpos);
						ret.push_back(split.substr((prevpos+1), (strsize-prevpos)));
					}
					prevpos = posdelim;
				}
				
			}
		}
		return ret;
	}

	inline std::string replaceAllChar(const std::string& str, char toreplace, const std::string& replacewith)
	{
		std::string res;
		std::vector<std::string> splitdelim = StringUtility::splitString(str, toreplace);
		for(unsigned int i = 0; i < splitdelim.size(); i++)
		{
			res += splitdelim.at(i);
			res += replacewith;
		}
		return res;
	}

	inline std::string substring(const std::string& str, unsigned int begin, unsigned int end)
	{
		unsigned int strsize = str.length();
		if(end > strsize)
			return "_";
		return str.substr((begin-1), (end-begin)+1);
	}
	
	inline std::string format(const std::vector<std::string>& split)
	{
		std::string ret = "[";
		for(unsigned int i = 0; i < split.size(); i++)
		{
			ret += split.at(i);
			if(i < (split.size() - 1))
				ret += ",";
			else
				ret += "]";
		}
		return ret;
	}
	
	inline std::vector<std::string> deformat(const std::string& str)
	{
		return StringUtility::splitString(StringUtility::replaceAllChar(StringUtility::replaceAllChar(str, '[', ""), ']', ""), ',');
	}
	
	inline Vector3F vectoriseList3F(const std::vector<std::string>& list)
	{
		if(list.size() < 3)
			return Vector3F();
		return Vector3F(CastUtility::fromString<float>(list.at(0)), CastUtility::fromString<float>(list.at(1)), CastUtility::fromString<float>(list.at(2)));
	}
	
	inline std::vector<std::string> devectoriseList3F(Vector3F v)
	{
		std::vector<std::string> ret;
		ret.reserve(3);
		ret.push_back(CastUtility::toString<float>(v.getX()));
		ret.push_back(CastUtility::toString<float>(v.getY()));
		ret.push_back(CastUtility::toString<float>(v.getZ()));
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