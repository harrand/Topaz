#ifndef UTILITY_HPP
#define UTILITY_HPP
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <unordered_map>
#include <limits>
#include <typeinfo>
#include <random>
#include <utility>
#include "vector.hpp"

namespace castutility
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

namespace stringutility
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
		return false;
	}
	
	inline std::vector<std::string> splitString(const std::string& s, const std::string& delims)
	{
		std::vector<std::string> v;
		// Start of an element.
		std::size_t elemStart = 0;
		// We start searching from the end of the previous element, which
		// initially is the start of the string.
		std::size_t elemEnd = 0;
		// Find the first non-delim, i.e. the start of an element, after the end of the previous element.
		while((elemStart = s.find_first_not_of(delims, elemEnd)) != std::string::npos)
		{
			// Find the first delem, i.e. the end of the element (or if this fails it is the end of the string).
			elemEnd = s.find_first_of(delims, elemStart);
			// Add it.
			v.emplace_back(s, elemStart, elemEnd == std::string::npos ? std::string::npos : elemEnd - elemStart);
		}
		// When there are no more non-spaces, we are done.
		return v;
	}
	
	inline std::vector<std::string> splitString(const std::string& s, char delim)
	{
		return splitString(s, castutility::toString(delim));
	}

	inline std::string replaceAllChar(const std::string& str, char toreplace, const std::string& replacewith)
	{
		std::string res;
		std::vector<std::string> splitdelim = stringutility::splitString(str, toreplace);
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
		return stringutility::splitString(stringutility::replaceAllChar(stringutility::replaceAllChar(str, '[', ""), ']', ""), ',');
	}
	
	template<typename T>
	inline Vector3<T> vectoriseList3(const std::vector<std::string>& list)
	{
		if(list.size() < 3)
			return Vector3<T>();
		return Vector3<T>(castutility::fromString<T>(list.at(0)), castutility::fromString<T>(list.at(1)), castutility::fromString<T>(list.at(2)));
	}
	
	template<typename T>
	inline std::vector<std::string> devectoriseList3(Vector3<T> v)
	{
		std::vector<std::string> ret;
		ret.reserve(3);
		ret.push_back(castutility::toString<T>(v.getX()));
		ret.push_back(castutility::toString<T>(v.getY()));
		ret.push_back(castutility::toString<T>(v.getZ()));
		return ret;
	}
}

namespace logutility
{
	inline void silent()
	{
		std::cout << "\n";
	}
	template<typename FirstArg, typename... Args>
	inline void silent(FirstArg arg, Args... args)
	{
		if(std::string(typeid(arg).name()) == "std::string")
			std::cout << arg;
		else
			std::cout << castutility::toString(arg);
		logutility::silent(args...);
	}
	
	template<typename FirstArg = void, typename... Args>
	inline void message(FirstArg arg, Args... args)
	{
		std::cout << "[Message]:\t";
		logutility::silent(arg, args...);
	}
	
	template<typename FirstArg = void, typename... Args>
	inline void warning(FirstArg arg, Args... args)
	{
		std::cout << "[Warning]:\t";
		logutility::silent(arg, args...);
	}
	
	template<typename FirstArg = void, typename... Args>
	inline void error(FirstArg arg, Args... args)
	{
		std::cout << "[Error]:\t";
		logutility::silent(arg, args...);
	}
}

class Force
{
public:
	Force(Vector3F size = Vector3F());
	Force(const Force& copy) = default;
	Force(Force&& move) = default;
	Force& operator=(const Force& rhs) = default;
	
	const Vector3F& getSize() const;
	void setSize(Vector3F size);
	Force operator+(const Force& other) const;
	Force operator-(const Force& other) const;
	Force operator*(float rhs) const;
	Force operator/(float rhs) const;
	Force& operator+=(const Force& other);
	Force& operator-=(const Force& other);
private:
	Vector3F size;
};

class Random
{
public:
	Random(std::default_random_engine::result_type seed = std::random_device()());
	Random(const Random& copy);
	Random(Random&& move) = default;
	Random& operator=(const Random& rhs) = default;
	
	const std::default_random_engine::result_type& getSeed() const;
	const std::default_random_engine& getEngine() const;
	std::default_random_engine& getEngineR();
	
	int nextInt(int min = 0, int max = std::numeric_limits<int>::max());
	float nextFloat(float min = 0, float max = std::numeric_limits<float>::max());
	
	template <typename Number>
	inline Number operator()(Number min, Number max)
	{
		static_assert(std::is_same<Number, float>::value || std::is_same<Number, int>::value, "Random::operator() must receive template arguments of float or int.");
		if(std::is_same<Number, float>::value)
			return nextFloat(min, max);
		else if(std::is_same<Number, int>::value)
			return nextInt(min, max);
	}
private:
	const std::default_random_engine::result_type seed;
	std::default_random_engine random_engine;
};

class MersenneTwister
{
public:
	MersenneTwister(std::mt19937::result_type seed = std::random_device()());
	MersenneTwister(const MersenneTwister& copy);
	MersenneTwister(MersenneTwister&& move) = default;
	MersenneTwister& operator=(const MersenneTwister& rhs) = default;
	
	const std::mt19937::result_type& getSeed() const;
	const std::mt19937& getEngine() const;
	std::mt19937& getEngineR();
	
	int nextInt(int min = 0, int max = std::numeric_limits<int>::max());
	float nextFloat(float min = 0, float max = std::numeric_limits<float>::max());
	
	template <typename Number>
	inline Number operator()(Number min, Number max)
	{
		static_assert(std::is_same<Number, float>::value || std::is_same<Number, int>::value, "MersenneTwister::operator() must receive template arguments of float or int.");
		if(std::is_same<Number, float>::value)
			return nextFloat(min, max);
		else if(std::is_same<Number, int>::value)
			return nextInt(min, max);
	}
private:
	const std::mt19937::result_type seed;
	std::mt19937 mersenne_twister_engine;
};

#endif //UTILITY_HPP