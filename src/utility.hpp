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
#include <functional>
#include <thread>
#include "MDL/mdl_file.hpp"
#include "vector.hpp"
#include "SDL2/SDL.h"

namespace tz
{	
	namespace consts
	{
		constexpr double pi = 4 * std::atan(1);
		constexpr double e = std::exp(1);
	}
	namespace literals
	{
		/**
		* Convert a mass in metric kilograms (kg) to imperial pounds (lb).
		*/
		inline long double operator""_lb(long double mass)
		{
			return mass * 0.45359237;
		}
		/**
		* Convert a mass in metric kilograms(kg) to imperial stone (st).
		*/
		inline long double operator""_st(long double mass)
		{
			using namespace tz::literals;
			return operator""_lb(mass * 14.0);
		}
		/**
		* Convert an angle in degrees to an angle in radians.
		* i.e: 180_deg = π
		*/
		inline long double operator""_deg(long double angle)
		{
			return angle * tz::consts::pi / 180.0;
		}
		/**
		* Convert an angle in radians to an angle in degrees.
		* i.e: π_rad = 180
		*/
		inline long double operator""_rad(long double angle)
		{
			return angle * 180.0 / tz::consts::pi;
		}
	}
	namespace util
	{
		template<typename Container>
		constexpr std::size_t sizeof_element(Container element_list);
		
		namespace cast
		{
			/**
			* Convert anything that can be converted into an std::string, into an std::string.
			*/
			template <typename T>
			inline std::string to_string(T&& obj);
			/**
			* Convert an std::string to any value, if it can.
			*/
			template <typename T>
			inline T from_string(const std::string& s);
		}
		
		/**
		* Perform processing on std::strings with these utility functions.
		*/
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
			inline std::string replace_all(std::string str, const std::string& to_replace, const std::string& replace_with);
			inline std::string substring(const std::string& str, std::size_t begin, std::size_t end);
			inline std::string format(const std::vector<std::string>& split);
			inline std::vector<std::string> deformat(const std::string& str);
			template<typename T>
			inline Vector3<T> vectorise_list_3(const std::vector<std::string>& list);
			template<typename T>
			inline std::vector<std::string> devectorise_list_3(Vector3<T> v);
		}

		/**
		* Log to the console variadically.
		* Like printf, but without the formatting and with type-safety.
		*/
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
		
		namespace scheduler
		{
			/**
			* Invokes std::functions synchronously (pretty much just runs a function for you) or asynchronously (runs the function in another thread as to not impede current processing).
			* You may well find this incredibly useful, however it does contain some overhead and therefore is not recommended for small, menial tasks.
			* For smaller and simpler tasks, it is highly recommended that you instead use tz::util::scheduler::[a]sync_delayed_functor(TrivialFunctor), in command.hpp.
			*/
			template<class ReturnType, class... Args>
			inline void sync_delayed_function(unsigned int milliseconds_delay, std::function<ReturnType(Args...)> f, Args... args);
			template<class ReturnType, class... Args>
			inline void async_delayed_function(unsigned int milliseconds_delay, std::function<ReturnType(Args...)> f, Args... args);
		}
			template<typename Number = int>
			inline Number random();
	}
}

/**
* Generate a random number using any of the C++ standard library random engines.
* Using default template arguments yields implementation-defined behaviour, but normally is a linear-congruentional engine.
*/
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

/**
* Template specialisation of Random, using the C++ mersenne-twister functionality.
* More expensive than a linear-congruentional approach, but does provide higher-quality pseudorandomness.
*/
using MersenneTwister = Random<std::mt19937, std::mt19937::result_type>;

/**
 * Wrapper for a function with variadic arguments. Unlike TrivialFunctor, cannot be inserted into a CommandExecutor.
 */
template<typename FunctorT>
class Functor
{
public:
	Functor(FunctorT functor);
	template<typename... FunctorParameters>
	void operator()(FunctorParameters... parameters);
private:
	FunctorT functor;
};
#include "utility.inl"
#endif