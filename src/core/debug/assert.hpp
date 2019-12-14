#ifndef ASSERT_HPP
#define ASSERT_HPP
#include <iostream>

namespace tz
{

#ifdef topaz_assert
#undef topaz_assert
#endif
/**
 * Evaluate the expression.
 * If TOPAZ_DEBUG == 1 and the expression evaluates to false, abort the runtime with the given message.
 */
#define topaz_assert(EXPRESSION, ...) ((EXPRESSION) ? \
(void)0 : tz::assert_message(std::cerr, \
"Assertion failure: ", #EXPRESSION, "\nIn file: ", __FILE__, \
" on line ", __LINE__, ":\n\t", __VA_ARGS__))

	template<typename... Args>
	inline void assert_message(std::ostream &out, Args &&... args)
	{
#if TOPAZ_DEBUG
		std::flush(out);
		(out << ... << args) << std::endl;
		std::abort();
#endif
	}
}

#endif // ASSERT_HPP