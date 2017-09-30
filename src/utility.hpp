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
#include "mdl.hpp"
#include "vector.hpp"
#include "SDL.h"

namespace tz
{	
	namespace consts
	{
		constexpr double pi = 4 * std::atan(1);
		constexpr double e = std::exp(1);
	}
	namespace util
	{
		namespace cast
		{
			template <typename T>
			inline std::string toString(T&& obj)
			{
				std::ostringstream oss;
				oss << std::forward<T>(obj);
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
		
		namespace string
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
		
			inline bool beginsWith(const std::string& what, const std::string& with_what)
			{
				return mdl::util::beginsWith(what, with_what);
			}
		
			inline bool endsWith(const std::string& what, const std::string& with_what)
			{
				return mdl::util::endsWith(what, with_what);
			}
		
			inline bool contains(const std::string& what, char withwhat)
			{
				const char* whatcstr = what.c_str();
				for(std::size_t i = 0; i < what.length(); i++)
					if(whatcstr[i] == withwhat)
						return true;
				return false;
			}
			
			inline std::vector<std::string> splitString(const std::string& s, const std::string& delim)
			{
				return mdl::util::splitString(s, delim);
			}
			
			inline std::vector<std::string> splitString(const std::string& s, char delim)
			{
				return splitString(s, tz::util::cast::toString(delim));
			}
		
			inline std::string replaceAllChar(const std::string& str, char toreplace, const std::string& replacewith)
			{
				std::string res;
				std::vector<std::string> splitdelim = tz::util::string::splitString(str, toreplace);
				for(std::size_t i = 0; i < splitdelim.size(); i++)
				{
					res += splitdelim[i];
					res += replacewith;
				}
				return res;
			}
		
			inline std::string substring(const std::string& str, unsigned int begin, unsigned int end)
			{
				std::size_t strsize = str.length();
				if(end > strsize)
					return "_";
				return str.substr((begin - 1), (end-begin) + 1);
			}
			
			inline std::string format(const std::vector<std::string>& split)
			{
				std::string ret = "[";
				for(std::size_t i = 0; i < split.size(); i++)
				{
					ret += split[i];
					if(i < (split.size() - 1))
						ret += ",";
					else
						ret += "]";
				}
				return ret;
			}
			
			inline std::vector<std::string> deformat(const std::string& str)
			{
				return tz::util::string::splitString(tz::util::string::replaceAllChar(tz::util::string::replaceAllChar(str, '[', ""), ']', ""), ',');
			}
			
			template<typename T>
			inline Vector3<T> vectoriseList3(const std::vector<std::string>& list)
			{
				if(list.size() < 3)
					return {};
				return {tz::util::cast::fromString<T>(list[0]), tz::util::cast::fromString<T>(list[1]), tz::util::cast::fromString<T>(list[2])};
			}
			
			template<typename T>
			inline std::vector<std::string> devectoriseList3(Vector3<T> v)
			{
				std::vector<std::string> ret;
				ret.reserve(3);
				ret.push_back(tz::util::cast::toString(v.getX()));
				ret.push_back(tz::util::cast::toString(v.getY()));
				ret.push_back(tz::util::cast::toString(v.getZ()));
				return ret;
			}
		}
		
		namespace log
		{
			inline void silent()
			{
				std::cout << "\n";
			}
			template<typename FirstArg, typename... Args>
			inline void silent(FirstArg arg, Args... args)
			{
				if constexpr(std::is_same<decltype(arg), std::string>::value)
					std::cout << arg;
				else
					std::cout << tz::util::cast::toString(arg);
				tz::util::log::silent(args...);
			}
			
			template<typename FirstArg = void, typename... Args>
			inline void message(FirstArg arg, Args... args)
			{
				std::cout << "[Message]:\t";
				tz::util::log::silent(arg, args...);
			}
			
			template<typename FirstArg = void, typename... Args>
			inline void warning(FirstArg arg, Args... args)
			{
				std::cout << "[Warning]:\t";
				tz::util::log::silent(arg, args...);
			}
			
			template<typename FirstArg = void, typename... Args>
			inline void error(FirstArg arg, Args... args)
			{
				std::cout << "[Error]:\t";
				tz::util::log::silent(arg, args...);
			}
		}
	}
}

class Force
{
public:
	Force(Vector3F size = Vector3F());
	Force(const Force& copy) = default;
	Force(Force&& move) = default;
	~Force() = default;
	Force& operator=(const Force& rhs) = default;
	
	const Vector3F& getSize() const;
	Vector3F& getSizeR();
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
	~Random() = default;
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
	~MersenneTwister() = default;
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

#endif