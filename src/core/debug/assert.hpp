#ifndef ASSERT_HPP
#define ASSERT_HPP
#include <iostream>
#include "core/debug/break.hpp"

namespace tz
{

	namespace debug
	{
		static bool assert_failure = false;
	}
	
#ifdef topaz_assert
#undef topaz_assert
#endif
/**
 * Evaluate the expression.
 * If TOPAZ_DEBUG == 1 and the expression evaluates to false, abort the runtime with the given message.
 */
#define topaz_assert(EXPRESSION, ...) ((EXPRESSION) ? \
(void)0 : tz::assert_message(std::cerr, false, \
"Assertion failure: ", #EXPRESSION, "\nIn file: ", __FILE__, \
" on line ", __LINE__, ":\n\t", __VA_ARGS__))

	template<typename... Args>
	inline void assert_message([[maybe_unused]] std::ostream &out, bool hard, [[maybe_unused]] Args &&... args)
	{
#if TOPAZ_DEBUG
		std::flush(out);
#ifdef TOPAZ_UNIT_TEST
		// Note: We don't print out the assertion message as a unit-test. It's expected that the expectation failure sort this out instead.
		debug::assert_failure = true;
#else
		// Use the given ostream.
		(out << ... << args) << std::endl;
		tz::debugbreak();
		if(hard)
		{
			out << "[HARD ASSERT DETETCED. ABORTING.]\n" << std::endl;
			std::abort();
		}
#endif
#endif
	}
	
	namespace debug::test
	{
		inline bool assert_failure()
		{
			return tz::debug::assert_failure;
		}
		
		inline void clear_assert_failure()
		{
			tz::debug::assert_failure = false;
		}
	}

#ifdef topaz_hard_assert
#undef topaz_hard_assert
#endif
#define topaz_hard_assert(EXPRESSION, ...) ((EXPRESSION) ? \
(void)0 : tz::assert_message(std::cerr, true, \
"[FATAL ERROR] Hard Assertion failure: ", #EXPRESSION, "\nIn file: ", __FILE__, \
" on line ", __LINE__, ":\n\t", __VA_ARGS__))
#ifdef topaz_assert_clear
#undef topaz_assert_clear
#endif
#define topaz_assert_clear() tz::debug::test::clear_assert_failure()
}

#endif // ASSERT_HPP