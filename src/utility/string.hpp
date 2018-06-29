#ifndef STRING_UTILITY_HPP
#define STRING_UTILITY_HPP
#include "generic.hpp"
#include <vector>

namespace tz::utility::string
{
    /**
     * Convert the input string to lower-case.
	 * @param data - The string to convert to lower-case
	 * @return - data, in lower-case
	 */
    std::string to_lower(std::string data);
    /**
     * Convert the input string to upper-case.
     * @param data - The string to convert to upper-case
     * @return - data, in upper-case
     */
    std::string to_upper(std::string data);
    /**
     * Query whether an existing string begins with another.
     * @param what - The string to query
     * @param with_what - The prefix to equate
     * @return - True if with starts with with_what. False otherwise
     */
    bool begins_with(const std::string& what, const std::string& with_what);
    /**
     * Query whether an existing string ends with another.
     * @param what - The string to query
     * @param with_what - The suffix to equate
     * @return - True if with ends with with_what. False otherwise
     */
    bool ends_with(const std::string& what, const std::string& with_what);
    /**
     * Query whether an existing string contains another substring.
     * @param what - The string to query
     * @param withwhat - The substring to query for containment
     * @return - True if what contains the substring withwhat. False otherwise
     */
    bool contains(const std::string& what, char withwhat);
    /**
     * Split an existing string with a specified delimiter.
     * @param s - The string to perform on
     * @param delim - The delimiter to use
     * @return - Container of strings, split from the source string via the specified delimiter
     */
    std::vector<std::string> split_string(const std::string& string, const std::string& delimiter);
    /**
     * Split an existing string with a specified delimiter.
     * @param s - The string to perform on
     * @param delim - The delimiter to use
     * @return - Container of strings, split from the source string via the specified delimiter
     */
    std::vector<std::string> split_string(const std::string& s, char delim);
    /**
     * Replace all instances of a character in a specified string with a replacement string.
     * @param str - The specified string
     * @param toreplace - The character to be replaced
     * @param replacewith - The replacement string
     * @return - The edited source string
     */
    std::string replace_all_char(const std::string& str, char toreplace, const std::string& replacewith);
    /**
     * Replace all instances of a string in a specified string with a replacement string.
     * @param str - The specified string
     * @param to_replace - The string to be replaced
     * @param replace_with - The replacement string
     * @return - The edited source string
     */
    std::string replace_all(std::string str, const std::string& to_replace, const std::string& replace_with);
    /**
     * Construct a substring from an existing string between two indices.
     * @param str - The source string
     * @param begin - The first index
     * @param end - The second index
     * @return - The substring
     */
    std::string substring(const std::string& str, std::size_t begin, std::size_t end);
}

#endif //STRING_UTILITY_HPP
