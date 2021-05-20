#ifndef TOPAZ_CORE_ASSERT_HPP
#define TOPAZ_CORE_ASSERT_HPP
#include "debugbreak.h"
#include <cstdio>
#include <cstdlib>
#include <utility>

namespace tz
{

#ifdef tz_assert
#undef tz_assert
#endif
#define tz_assert(EXPRESSION, fmt, ...) ((EXPRESSION) ? \
(void)0 : tz::assert_message(stderr, false, \
"Assertion failure: %s\nIn file: %s on line %d:\n\t" fmt, #EXPRESSION, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__))

    template<typename... Args>
    inline void assert_message([[maybe_unused]] FILE* output_stream, [[maybe_unused]] bool hard, [[maybe_unused]] const char* fmt, [[maybe_unused]] Args&&... args)
    {
#if TZ_DEBUG
        // Use the given ostream.
        fflush(output_stream);
        fprintf(output_stream, fmt, std::forward<Args>(args)...);
        fflush(output_stream);
        ::debug_break();
        if(hard)
        {
            fprintf(output_stream, "%s", "[HARD ASSERT DETECTED. ABORTING.]\n");
            std::abort();
        }
#endif
    }

#define tz_error(fmt, ...) (tz::error_message(stderr, \
"Error (tz_error): \nIn file: %s on line %d:\n\t" fmt, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__))


    template<typename... Args>
    inline void error_message([[maybe_unused]] FILE* output_stream, [[maybe_unused]] const char* fmt, [[maybe_unused]] Args&&... args)
    {
#if TZ_DEBUG
        // Use the given ostream.
        fflush(output_stream);
        fprintf(output_stream, fmt, std::forward<Args>(args)...);
        fflush(output_stream);
        ::debug_break();
#endif
    }
}

#endif // TOPAZ_CORE_ASSERT_HPP