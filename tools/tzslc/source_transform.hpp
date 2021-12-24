#ifndef TZSLC_SOURCE_TRANSFORM_HPP
#define TZSLC_SOURCE_TRANSFORM_HPP
#include "core/types.hpp"
#include "core/assert.hpp"
#include <string>
#include <vector>
#include <regex>

namespace tzslc
{
	namespace detail
	{
		using SourceMatchIteratorType = std::vector<std::string>::iterator;
	}
	template<typename F>
	concept SourceReplaceAction = tz::Function<F, std::string, detail::SourceMatchIteratorType, detail::SourceMatchIteratorType>;
	bool transform(std::string& source, std::regex reg, SourceReplaceAction auto transform_function)
	{
		using ReplaceJob = std::pair<std::pair<std::size_t, std::size_t>, std::string>;
		std::vector<ReplaceJob> replacements;
		
		std::string src_copy = source;
		std::smatch sm;
		std::size_t src_pos_counter = 0;

		bool did_any_work = false;

		while(std::regex_search(src_copy, sm, reg))
		{
			// Found a match!
			std::size_t pos = sm.position() + src_pos_counter;
			tz_assert(pos < source.size(), "Match result has position %zu in source which is out of range. Size: %zu", pos, source.size());
			std::size_t len = sm.length();
			std::vector<std::string> inner_matches;
			for(std::size_t i = 1; i < sm.size(); i++)
				inner_matches.push_back(sm[i]);
			src_copy = sm.suffix();
			src_pos_counter = source.find(src_copy);
			//src_pos_counter += pos + len;
			// get the transformed string.
			std::string replacement = transform_function(inner_matches.begin(), inner_matches.end());
			// register this source replacement.
			ReplaceJob job{{pos, len}, replacement};
			replacements.push_back(job);
		}

		did_any_work = !replacements.empty();

		// Invoke all replacements (in reverse-order)
		for(auto i = replacements.rbegin(); i != replacements.rend(); i++)
		{
			std::string replacement = i->second;
			std::size_t pos = i->first.first;
			std::size_t len = i->first.second;
			source.replace(pos, len, replacement);
		}

		return did_any_work;
	}
}

#endif // TZSLC_SOURCE_TRANSFORM_HPP
