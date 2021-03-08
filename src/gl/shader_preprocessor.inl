#include "core/debug/assert.hpp"

namespace tz::gl
{
	namespace src
	{
		template<typename Runnable>
		void transform(std::string& source, std::regex reg, Runnable transform_function)
		{
			using ReplaceJob = std::pair<std::pair<std::size_t, std::size_t>, std::string>;
			std::vector<ReplaceJob> replacements;
			
			std::string src_copy = source;
			std::smatch sm;
			std::size_t src_pos_counter = 0;

			while(std::regex_search(src_copy, sm, reg))
			{
				// Found a match!
				std::size_t pos = sm.position() + src_pos_counter;
				topaz_assert(pos < source.size(), "tz::gl::src::transform(...): Match result has position ", pos, " in source which is out of range. Size: ", source.size());
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

			// Invoke all replacements (in reverse-order)
			for(auto i = replacements.rbegin(); i != replacements.rend(); i++)
			{
				std::string replacement = i->second;
				std::size_t pos = i->first.first;
				std::size_t len = i->first.second;
				source.replace(pos, len, replacement);
			}
		}
	}

	template<typename ModuleT, typename... Args>
	std::size_t ShaderPreprocessor::emplace_module(Args&&... args)
	{
		std::size_t idx = this->modules.size();
		this->modules.push_back(std::make_unique<ModuleT>(std::forward<Args>(args)...));
		return idx;
	}
}