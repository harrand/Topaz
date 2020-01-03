#include "gl/modules/include.hpp"
#include "core/debug/assert.hpp"
#include <regex>
#include <fstream>
#include <sstream>

namespace tz::gl::p
{
	IncludeModule::IncludeModule(std::string source_path): path(source_path){}

	void IncludeModule::operator()(std::string& source) const
	{
		src::transform(source, std::regex{"#include \"(.+)\""}, [this](auto beg, auto end)
		{
	        topaz_assert(std::distance(beg, end) == 1, "tz::gl::p::IncludeModule::operator(): Had unexpected number of inner matches. Expected ", 1, ", got ", std::distance(beg, end));
			std::string include_file_name = *beg;
			return this->cat_include(include_file_name);
		});
	}

	std::string IncludeModule::cat_include(std::string include_path) const
	{
		// Looks like std::filesystem is completely broken for my compiler. I'll just work with paths manually...
		std::string parent_path = this->path;
		if(parent_path.back() != '/' || parent_path.back() != '\\')
		{
			// if it has no trailing slash, add one.
			parent_path += '/';
		}
		parent_path += "../"; // Now we're in the parent directory.
		std::string full_include_path = parent_path + include_path;
		std::ifstream include_file(full_include_path);
		topaz_assert(include_file.good(), "tz::gl::p::IncludModule::cat_include(", include_path, "): Couldn't read the file. Relative Path: ", full_include_path);
		std::stringstream ss;
		ss << include_file.rdbuf();
		return ss.str();
	}
}
