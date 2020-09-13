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
 * Evaluate the expression. This should be used to guarantee that expressions are true at runtime.
 * If an assertion failure is catastrophic for the machine (such as a GPU/BSOD), use a hard-assert instead.
 * If the expression can be evaluated at compile-time, use a static_assert -- Guaranteed by the C++ standard since C++11.
 * If TOPAZ_DEBUG == 1 and the expression evaluates to false, send an error message to cerr and send a breakpoint signal to the debugger. If no debugger is attached, the OS will likely kill the program.
 * Note: To guarantee termination on failure without a debugger, use a hard-assert.
 */
#define topaz_assert(EXPRESSION, ...) ((EXPRESSION) ? \
(void)0 : tz::assert_message(std::cerr, false, \
"Assertion failure: ", #EXPRESSION, "\nIn file: ", __FILE__, \
" on line ", __LINE__, ":\n\t", __VA_ARGS__))

	template<typename... Args>
	inline void assert_message([[maybe_unused]] std::ostream &out, [[maybe_unused]] bool hard, [[maybe_unused]] Args &&... args)
	{
#if TOPAZ_DEBUG
		std::flush(out);
		#if TOPAZ_UNIT_TEST
			bool unit_test = true;
		#else
			bool unit_test = false;
		#endif
		if(unit_test)
		{
			// Note: We don't print out the assertion message as a unit-test. It's expected that the expectation failure sort this out instead.
			debug::assert_failure = true;
		}
		else
		{
			// Use the given ostream.
			(out << ... << args) << std::endl;
			tz::debugbreak();
			if(hard)
			{
				out << "[HARD ASSERT DETECTED. ABORTING.]\n" << std::endl;
				std::abort();
			}
		}
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
/**
 * Evaluate the expression in a stricter manner than a normal assert.
 * If TOPAZ_DEBUG == 1 and the expression evaluates to false, send an error message to cerr and send a breakpoint signal to the debugger. After that, invoke abort() to guarantee program termination.
 */
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