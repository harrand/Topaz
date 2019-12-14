//
// Created by Harrand on 13/12/2019.
//

#ifndef TOPAZ_PRINT_HPP
#define TOPAZ_PRINT_HPP

namespace tz
{
	/**
	 * Print formatted data to the console if TOPAZ_DEBUG == 1
	 * Nothing more than a conditional printf. For an unconditional printf, just use printf!
	 * @tparam Args - Argument types to print
	 * @param fmt - Format string
	 * @param args - Argument values to print
	 */
	template<typename... Args>
	void debug_printf(const char* fmt, Args&&... args);
}

#include "core/debug/print.inl"
#endif //TOPAZ_PRINT_HPP
