//
// Created by Harry on 27/12/2018.
//

#ifndef TOPAZ_LOG_HPP
#define TOPAZ_LOG_HPP
#include <type_traits>

namespace tz::debug
{
	inline void print(){}

	template<typename FirstArg, typename... Args>
	inline void print(FirstArg&& arg, Args&&... args)
	{
		#ifndef TOPAZ_DEBUG
			return;
		#endif
		if constexpr(std::is_same_v<FirstArg, std::string>)
			std::cout << arg;
		else
			std::cout << tz::utility::generic::cast::to_string(arg);
		tz::debug::print(std::forward<Args>(args)...);
	}
}

#endif //TOPAZ_LOG_HPP
