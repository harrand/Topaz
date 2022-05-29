#include "core/assert.hpp"
#include <cstdint>

namespace tz
{
	constexpr std::int32_t static_stoi(std::string_view str, std::size_t* pos = nullptr);

	constexpr Version parse_version(const char* version_string)
	{
		if(version_string == nullptr || version_string[0] == '\0')
		{
			return {.major = 0, .minor = 0, .patch = 0};
		}
		std::string_view version_sv = version_string;
		Version ver;
		using spos = std::string_view::size_type;
		spos max = {}, min = {}, patch = {};
		unsigned int dot_counter = 0, counter = 0;
		for(char c : version_sv)
		{
			if(c == '.')
			{
				switch(dot_counter)
				{
					case 0:
						max = counter;
					break;
					case 1:
						min = counter;
						patch = min + 1;
					break;
				}
				dot_counter++;
			}
			counter++;
		}

		ver.major = static_stoi(version_sv.substr(0, max));
		ver.minor = static_stoi(version_sv.substr(max + 1, min));
		ver.patch = static_stoi(version_sv.substr(min + 1, patch));
		return ver;
	}

	constexpr std::int32_t static_stoi(std::string_view str, std::size_t* pos)
	{
		using namespace std::literals;
		const auto numbers = "0123456789"sv;

		const auto begin = str.find_first_of(numbers);
		if (begin == std::string_view::npos)
		{
			tz_error("tz::static_stoi(%.*s, ...): Failed to convert to a number.", str.size(), str.data());
			return -1;
		}
			
		const auto sign = begin != 0U && str[begin - 1U] == '-' ? -1 : 1;
		str.remove_prefix(begin);

		const auto end = str.find_first_not_of(numbers);
		if (end != std::string_view::npos)
			str.remove_suffix(std::size(str) - end);

		auto result = 0;
		auto multiplier = 1U;
		for (std::ptrdiff_t i = std::size(str) - 1U; i >= 0; --i) {
			auto number = str[i] - '0';
			result += number * multiplier * sign;
			multiplier *= 10U;
		}

		if (pos != nullptr) *pos = begin + std::size(str);
		return result;
	}
}
