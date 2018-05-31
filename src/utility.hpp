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
#include "vector.hpp"
#include "SDL2/SDL.h"

namespace tz
{	
	namespace consts
	{
		/// 3.14159...
		constexpr double pi = 4 * std::atan(1);
		/// 2.17...
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
		/**
		 * Given a container, find the size, in bytes, of an element in the container.
		 * @tparam Container - Container type.
		 * @param element_list - Container of elements.
		 * @return
		 */
		template<typename Container>
		constexpr std::size_t sizeof_element(Container element_list);
		
		namespace cast
		{
			/**
			 * (Attempt to) Convert an object to a std::string.
			 * @tparam T - Type of the object to convert
			 * @param obj - The object to convert
			 * @return - The object, converted to a string
			 */
			template <typename T>
			inline std::string to_string(T&& obj);
			/**
			 * (Attempt to) Convert an std::string to an object of specified type.
			 * @tparam T - Type of the object to convert to
			 * @param s - The object to convert to
			 * @return - The object that the string was converted to
			 */
			template <typename T>
			inline T from_string(const std::string& s);
		}

		/** Perform simple file-io
		 *
		 */
		namespace file
		{
			inline std::string read(const std::string& path);
		}
		
		/**
		* Perform processing on std::strings with these utility functions.
		*/
		namespace string
		{
		    /**
		     * Convert the input string to lower-case.
		     * @param data - The string to convert to lower-case
		     * @return - data, in lower-case
		     */
			inline std::string to_lower(std::string data);
            /**
             * Convert the input string to upper-case.
             * @param data - The string to convert to upper-case
             * @return - data, in upper-case
             */
			inline std::string to_upper(std::string data);
			/**
			 * Query whether an existing string begins with another.
			 * @param what - The string to query
			 * @param with_what - The prefix to equate
			 * @return - True if with starts with with_what. False otherwise
			 */
			inline bool begins_with(const std::string& what, const std::string& with_what);
            /**
             * Query whether an existing string ends with another.
             * @param what - The string to query
             * @param with_what - The suffix to equate
             * @return - True if with ends with with_what. False otherwise
             */
			inline bool ends_with(const std::string& what, const std::string& with_what);
			/**
			 * Query whether an existing string contains another substring.
			 * @param what - The string to query
			 * @param withwhat - The substring to query for containment
			 * @return - True if what contains the substring withwhat. False otherwise
			 */
			inline bool contains(const std::string& what, char withwhat);
			/**
			 * Split an existing string with a specified delimiter.
			 * @param s - The string to perform on
			 * @param delim - The delimiter to use
			 * @return - Container of strings, split from the source string via the specified delimiter
			 */
			inline std::vector<std::string> split_string(const std::string& string, const std::string& delimiter);
            /**
             * Split an existing string with a specified delimiter.
             * @param s - The string to perform on
             * @param delim - The delimiter to use
             * @return - Container of strings, split from the source string via the specified delimiter
             */
			inline std::vector<std::string> split_string(const std::string& s, char delim);
			/**
			 * Replace all instances of a character in a specified string with a replacement string.
			 * @param str - The specified string
			 * @param toreplace - The character to be replaced
			 * @param replacewith - The replacement string
			 * @return - The edited source string
			 */
			inline std::string replace_all_char(const std::string& str, char toreplace, const std::string& replacewith);
			/**
			 * Replace all instances of a string in a specified string with a replacement string.
			 * @param str - The specified string
			 * @param to_replace - The string to be replaced
			 * @param replace_with - The replacement string
			 * @return - The edited source string
			 */
			inline std::string replace_all(std::string str, const std::string& to_replace, const std::string& replace_with);
			/**
			 * Construct a substring from an existing string between two indices.
			 * @param str - The source string
			 * @param begin - The first index
			 * @param end - The second index
			 * @return - The substring
			 */
			inline std::string substring(const std::string& str, std::size_t begin, std::size_t end);
			/**
			 * Emplace all elements of a container of strings within the following string:
			 * [element0, element1, etc...]
			 * @param split - The container of strings to format
			 * @return - The formatted container of strings
			 */
			inline std::string format(const std::vector<std::string>& split);
			/**
			 * Given a formatted string, extract all the elements contained within the formatted string.
			 * @param str - The formatted string
			 * @return - Container of the strings in the formatted string
			 */
			inline std::vector<std::string> deformat(const std::string& str);
			/**
			 * Convert the first three elements of a container of strings into the type T, and place into a 3-dimensional Vector.
			 * @tparam T - The type to store into the 3-dimensional Vector
			 * @param list - The container of strings
			 * @return - The resultant 3-dimensional Vector
			 */
			template<typename T>
			inline Vector3<T> vectorise_list_3(const std::vector<std::string>& list);
			/**
			 * Given a 3-dimensional Vector containing types T, convert them into a string and place within a container.
			 * @tparam T - Type of the elements in the 3-dimensional Vector
			 * @param v - The 3-dimensional Vector to extract from
			 * @return - The resultant container of strings
			 */
			template<typename T>
			inline std::vector<std::string> devectorise_list_3(Vector3<T> v);
		}

		/**
		* Log to the console variadically.
		* Like printf, but without the formatting and with type-safety.
		*/
		namespace log
		{
		    /// Base-case for log::silent, log::message, log::warning and log::error.
			inline void silent();
			/**
			 * Essentially a printf but without formatting. Prints the parameters to the standard output.
			 * @tparam FirstArg - Type of the first argument
			 * @tparam Args - Type of the remaining arguments
			 * @param arg - The first argument
			 * @param args - The remaining arguments
			 */
			template<typename FirstArg, typename... Args>
			inline void silent(FirstArg arg, Args... args);
			/**
			 * Print the parameters with Topaz-formatting to the standard output.
			 * Output: [Message]: parameters
			 * @tparam FirstArg - Type of the first argument
			 * @tparam Args - Type of the remaining arguments
			 * @param arg - The first argument
			 * @param args - The remaining arguments
			 */
			template<typename FirstArg = void, typename... Args>
			inline void message(FirstArg arg, Args... args);
            /**
             * Print the parameters with Topaz-formatting to the standard output.
             * Output: [Warning]: parameters
             * @tparam FirstArg - Type of the first argument
             * @tparam Args - Type of the remaining arguments
             * @param arg - The first argument
             * @param args - The remaining arguments
             */
			template<typename FirstArg = void, typename... Args>
			inline void warning(FirstArg arg, Args... args);
            /**
             * Print the parameters with Topaz-formatting to the standard output.
             * Output: [Error]: parameters
             * @tparam FirstArg - Type of the first argument
             * @tparam Args - Type of the remaining arguments
             * @param arg - The first argument
             * @param args - The remaining arguments
             */
			template<typename FirstArg = void, typename... Args>
			inline void error(FirstArg arg, Args... args);
		}
		
		namespace scheduler
		{
			/**
			 * Invokes a function synchronously with specified arguments after a specified delay.
			 * @tparam ReturnType - Return type of the function
			 * @tparam Args - Argument types of the function
			 * @param milliseconds_delay - Number of milliseconds to elapse before executing the function.
			 * @param f - The function to execute
			 * @param args - Arguments to emplace into the function invocation
			 */
			template<class ReturnType, class... Args>
			inline void sync_delayed_function(unsigned int milliseconds_delay, std::function<ReturnType(Args...)> f, Args... args);
            /**
             * Invokes a function asynchronously with specified arguments after a specified delay.
             * @tparam ReturnType - Return type of the function
             * @tparam Args - Argument types of the function
             * @param milliseconds_delay - Number of milliseconds to elapse before executing the function.
             * @param f - The function to execute
             * @param args - Arguments to emplace into the function invocation
             */
			template<class ReturnType, class... Args>
			inline void async_delayed_function(unsigned int milliseconds_delay, std::function<ReturnType(Args...)> f, Args... args);
		}

		/**
		 * Generate a random number.
		 * @tparam Number - Type of the number
		 * @return - The random number generated
		 */
		template<typename Number = int>
        inline Number random();
	}
}

/**
 * Object to generate random numbers with a given RNG-engine.
 * @tparam Engine - Random engine device
 * @tparam EngineResultType - Return type of the engine's productions
 */
template<typename Engine = std::default_random_engine, typename EngineResultType = std::default_random_engine::result_type>
class Random
{
public:
    /**
     * Generate a Random from a seed.
     * @param seed - The seed for the RNG engine
     */
	Random<Engine, EngineResultType>(EngineResultType seed = std::random_device()());
	/**
	 * Construct a Random from an existing, using their seed and engine.
	 * @param copy - Random object to copy the seed and engine from.
	 */
	Random<Engine, EngineResultType>(const Random<Engine, EngineResultType>& copy);

	/**
	 * Get the value of the seed to this engine.
	 * @return - The seed value
	 */
	const EngineResultType& get_seed() const;
	/**
	 * Read-only access to the underlying random engine.
	 * @return - The random engine being used
	 */
	const Engine& get_engine() const;
	/**
	 * Generate a random signed integer between specified limits.
	 * @param min - The minimum result of the integer
	 * @param max - The maximum result of the integer
	 * @return
	 */
	int next_int(int min = 0, int max = std::numeric_limits<int>::max());
    /**
     * Generate a random float between specified limits.
     * @param min - The minimum result of the float
     * @param max - The maximum result of the float
     * @return
     */
	float next_float(float min = 0, float max = std::numeric_limits<float>::max());
    /**
     * Generate a random number between specified limits.
     * @tparam Number - The type of value to generate
     * @param min - The minimum result of the number
     * @param max - The maximum result of the number
     * @return
     */
	template <typename Number = int>
	inline Number operator()(Number min = Number(), Number max = std::numeric_limits<Number>::max());
private:
    /// Stores the seed used for this Random object.
	const EngineResultType seed;
    /// Stores the underlying RNG engine for this Random object.
	Engine random_engine;
};

/**
* Template specialisation of Random, using the C++ mersenne-twister functionality.
* More expensive than a linear-congruentional approach, but does provide higher-quality pseudorandomness.
*/
using MersenneTwister = Random<std::mt19937, std::mt19937::result_type>;

/**
 * Wrapper for a Functor, using variadic arguments.
 * @tparam FunctorT - Type of the functor
 */
template<typename FunctorT>
class Functor
{
public:
    /**
     * Generate a Functor directly from a callable type.
     * @param functor - The functor value
     */
	Functor(FunctorT functor);
	/**
	 * Execute the functor, providing all parameter values
	 * @tparam FunctorParameters - Types of the functor parameters
	 * @param parameters - Values of the functor parameters
	 */
	template<typename... FunctorParameters>
	void operator()(FunctorParameters... parameters);
private:
    /// The underlying functor.
	FunctorT functor;
};
#include "utility.inl"
#endif