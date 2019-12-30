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
		using IncludeJob = std::pair<std::pair<std::size_t, std::size_t>, std::string>;
		std::vector<IncludeJob> include_processing_list;
		// Firstly populate processing jobs.
		{
			std::string src_copy = source;
			// TODO: Do include magic!
			std::regex reg{"#include \"(.+)\""};
			std::smatch sm;
			std::ptrdiff_t src_pos_counter = 0;
			while(std::regex_search(src_copy, sm, reg))
			{
				// Found something!
				std::ptrdiff_t pos = sm.position() + src_pos_counter;
				topaz_assert(pos < source.size(), "tz::gl::p::IncludeModule::operator(...): Match result has position ", pos, " in source which is out of range. Size: ", source.size());
				std::ptrdiff_t len = sm.length();
				std::string include_file_name = sm[1];
				src_copy = sm.suffix();
				src_pos_counter += pos + len;
				// Register an include process.
				std::pair<std::size_t, std::size_t> pos_and_length{pos, len};
				std::string include_data = this->cat_include(include_file_name);
				IncludeJob job{pos_and_length, include_data};
				include_processing_list.push_back(job);
			}
		}

		// Now invoke all jobs in reverse-order.
		for(auto i = include_processing_list.rbegin(); i != include_processing_list.rend(); i++)
		{
			std::string replacement = i->second;
			std::size_t pos = i->first.first;
			std::size_t len = i->first.second;
			source.replace(pos, len, replacement);
		}
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
