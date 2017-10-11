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
	namespace literals
	{
		// to encourage sticking to SI units and radians
		inline long double operator""_lb(long double mass)// return angle in kilograms
		{
			return mass * 0.45359237;
		}
		inline long double operator""_st(long double mass)// return angle in kilograms
		{
			using namespace tz::literals;
			return operator""_lb(mass * 14.0);
		}
		inline long double operator""_deg(long double angle)// return angle in radians
		{
			return angle * tz::consts::pi / 180.0;
		}
	}
	namespace util
	{
		namespace cast
		{
			template <typename T>
			inline std::string to_string(T&& obj)
			{
				std::ostringstream oss;
				oss << std::forward<T>(obj);
				return oss.str();
			}
			
			template <typename T>
			inline T from_string(const std::string& s)
			{
				T ret;
				std::istringstream ss(s);
				ss >> ret;
				return ret;
			}
		}
		
		namespace string
		{
			inline std::string to_lower(std::string data)
			{
				std::transform(data.begin(), data.end(), data.begin(), ::tolower);
				return data;
			}
		
			inline std::string to_upper(std::string data)
			{
				std::transform(data.begin(), data.end(), data.begin(), ::toupper);
				return data;
			}
		
			inline bool begins_with(const std::string& what, const std::string& with_what)
			{
				return mdl::util::begins_with(what, with_what);
			}
		
			inline bool ends_with(const std::string& what, const std::string& with_what)
			{
				return mdl::util::ends_with(what, with_what);
			}
		
			inline bool contains(const std::string& what, char withwhat)
			{
				const char* whatcstr = what.c_str();
				for(std::size_t i = 0; i < what.length(); i++)
					if(whatcstr[i] == withwhat)
						return true;
				return false;
			}
			
			inline std::vector<std::string> split_string(const std::string& s, const std::string& delim)
			{
				return mdl::util::split_string(s, delim);
			}
			
			inline std::vector<std::string> split_string(const std::string& s, char delim)
			{
				return split_string(s, tz::util::cast::to_string(delim));
			}
		
			inline std::string replace_all_char(const std::string& str, char toreplace, const std::string& replacewith)
			{
				std::string res;
				std::vector<std::string> splitdelim = tz::util::string::split_string(str, toreplace);
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
				return tz::util::string::split_string(tz::util::string::replace_all_char(tz::util::string::replace_all_char(str, '[', ""), ']', ""), ',');
			}
			
			template<typename T>
			inline Vector3<T> vectorise_list_3(const std::vector<std::string>& list)
			{
				if(list.size() < 3)
					return {};
				return {tz::util::cast::from_string<T>(list[0]), tz::util::cast::from_string<T>(list[1]), tz::util::cast::from_string<T>(list[2])};
			}
			
			template<typename T>
			inline std::vector<std::string> devectorise_list_3(Vector3<T> v)
			{
				std::vector<std::string> ret;
				ret.reserve(3);
				ret.push_back(tz::util::cast::to_string(v.get_x()));
				ret.push_back(tz::util::cast::to_string(v.get_y()));
				ret.push_back(tz::util::cast::to_string(v.get_z()));
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
					std::cout << tz::util::cast::to_string(arg);
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

class Random
{
public:
	Random(std::default_random_engine::result_type seed = std::random_device()());
	Random(const Random& copy);
	Random(Random&& move) = default;
	~Random() = default;
	Random& operator=(const Random& rhs) = default;
	
	const std::default_random_engine::result_type& get_seed() const;
	const std::default_random_engine& get_engine() const;
	int next_int(int min = 0, int max = std::numeric_limits<int>::max());
	float next_float(float min = 0, float max = std::numeric_limits<float>::max());
	template <typename Number>
	inline Number operator()(Number min, Number max)
	{
		static_assert(std::is_same<Number, float>::value || std::is_same<Number, int>::value, "Random::operator() must receive template arguments of float or int.");
		if(std::is_same<Number, float>::value)
			return next_float(min, max);
		else if(std::is_same<Number, int>::value)
			return next_int(min, max);
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
	
	const std::mt19937::result_type& get_seed() const;
	const std::mt19937& get_engine() const;
	int next_int(int min = 0, int max = std::numeric_limits<int>::max());
	float next_float(float min = 0, float max = std::numeric_limits<float>::max());
	template <typename Number>
	inline Number operator()(Number min, Number max)
	{
		static_assert(std::is_same<Number, float>::value || std::is_same<Number, int>::value, "MersenneTwister::operator() must receive template arguments of float or int.");
		if(std::is_same<Number, float>::value)
			return next_float(min, max);
		else if(std::is_same<Number, int>::value)
			return next_int(min, max);
	}
private:
	const std::mt19937::result_type seed;
	std::mt19937 mersenne_twister_engine;
};

#endif