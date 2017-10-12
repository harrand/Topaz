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
			inline std::string to_string(T&& obj);
			template <typename T>
			inline T from_string(const std::string& s);
		}
		
		namespace string
		{
			inline std::string to_lower(std::string data);
			inline std::string to_upper(std::string data);
			inline bool begins_with(const std::string& what, const std::string& with_what);
			inline bool ends_with(const std::string& what, const std::string& with_what);
			inline bool contains(const std::string& what, char withwhat);
			inline std::vector<std::string> split_string(const std::string& s, const std::string& delim);
			inline std::vector<std::string> split_string(const std::string& s, char delim);
			inline std::string replace_all_char(const std::string& str, char toreplace, const std::string& replacewith);
			inline std::string substring(const std::string& str, unsigned int begin, unsigned int end);
			inline std::string format(const std::vector<std::string>& split);
			inline std::vector<std::string> deformat(const std::string& str);
			template<typename T>
			inline Vector3<T> vectorise_list_3(const std::vector<std::string>& list);
			template<typename T>
			inline std::vector<std::string> devectorise_list_3(Vector3<T> v);
		}
		
		namespace log
		{
			inline void silent();
			template<typename FirstArg, typename... Args>
			inline void silent(FirstArg arg, Args... args);
			template<typename FirstArg = void, typename... Args>
			inline void message(FirstArg arg, Args... args);
			template<typename FirstArg = void, typename... Args>
			inline void warning(FirstArg arg, Args... args);
			template<typename FirstArg = void, typename... Args>
			inline void error(FirstArg arg, Args... args);
		}
	}
}

template<typename Engine = std::default_random_engine, typename EngineResultType = std::default_random_engine::result_type>
class Random
{
public:
	Random<Engine, EngineResultType>(EngineResultType seed = std::random_device()());
	Random<Engine, EngineResultType>(const Random<Engine, EngineResultType>& copy);
	Random<Engine, EngineResultType>(Random<Engine, EngineResultType>&& move) = default;
	~Random<Engine, EngineResultType>() = default;
	Random<Engine, EngineResultType>& operator=(const Random<Engine, EngineResultType>& rhs) = default;
	
	const EngineResultType& get_seed() const;
	const Engine& get_engine() const;
	int next_int(int min = 0, int max = std::numeric_limits<int>::max());
	float next_float(float min = 0, float max = std::numeric_limits<float>::max());
	template <typename Number = int>
	inline Number operator()(Number min = Number(), Number max = std::numeric_limits<Number>::max());
private:
	const EngineResultType seed;
	Engine random_engine;
};

using MersenneTwister = Random<std::mt19937, std::mt19937::result_type>;
#include "utility.inl"
#endif