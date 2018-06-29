#include "string.hpp" // tz::utility::string definitions
#include <algorithm>

namespace tz::utility::string
{
    std::string to_lower(std::string data)
    {
        std::transform(data.begin(), data.end(), data.begin(), ::tolower);
        return data;
    }

    std::string to_upper(std::string data)
    {
        std::transform(data.begin(), data.end(), data.begin(), ::toupper);
        return data;
    }

    bool begins_with(const std::string& what, const std::string& with_what)
    {
        return what.compare(0, with_what.length(), with_what) == 0;
    }

    bool ends_with(const std::string& what, const std::string& with_what)
    {
        if(what.length() >= with_what.length())
            return (0 == what.compare(what.length() - with_what.length(), with_what.length(), with_what));
        else
            return false;
    }

    bool contains(const std::string& what, char withwhat)
    {
        return what.find(withwhat) != std::string::npos;
    }

    std::vector<std::string> split_string(const std::string& string, const std::string& delimiter)
    {
        std::vector<std::string> v;
        // Start of an element.
        std::size_t element_start = 0;
        // We start searching from the end of the previous element, which
        // initially is the start of the string.
        std::size_t element_end = 0;
        // Find the first non-delim, i.e. the start of an element, after the end of the previous element.
        while((element_start = string.find_first_not_of(delimiter, element_end)) != std::string::npos)
        {
            // Find the first delem, i.e. the end of the element (or if this fails it is the end of the string).
            element_end = string.find_first_of(delimiter, element_start);
            // Add it.
            v.emplace_back(string, element_start, element_end == std::string::npos ? std::string::npos : element_end - element_start);
        }
        // When there are no more non-spaces, we are done.
        return v;
    }

    std::vector<std::string> split_string(const std::string& s, char delim)
    {
        return split_string(s, tz::utility::generic::cast::to_string(delim));
    }

    std::string replace_all_char(const std::string& str, char toreplace, const std::string& replacewith)
    {
        std::string res;
        std::vector<std::string> splitdelim = tz::utility::string::split_string(str, toreplace);
        for(std::size_t i = 0; i < splitdelim.size(); i++)
        {
            res += splitdelim[i];
            res += replacewith;
        }
        return res;
    }

    std::string replace_all(std::string str, const std::string& to_replace, const std::string& replace_with)
    {
        std::size_t pos = 0;
        while ((pos = str.find(to_replace, pos)) != std::string::npos)
        {
            str.replace(pos, to_replace.length(), replace_with);
            pos += replace_with.length();
        }
        return str;
    }

    std::string substring(const std::string& str, std::size_t begin, std::size_t end)
    {
        std::size_t strsize = str.length();
        if(end > strsize)
            return "_";
        return str.substr((begin - 1), (end-begin) + 1);
    }
}