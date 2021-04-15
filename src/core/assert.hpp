#ifndef TOPAZ_CORE_ASSERT_HPP
#define TOPAZ_CORE_ASSERT_HPP
#include "debugbreak.h"
#include <cstdio>

namespace tz
{

#ifdef tz_assert
#undef tz_assert
#endif
#define tz_assert(EXPRESSION, fmt, ...) ((EXPRESSION) ? \
(void)0 : tz::assert_message(stderr, false, \
"Assertion failure: %s\nIn file: %s on line %d:\n\t" fmt, #EXPRESSION, __FILE__, __LINE__, __VA_ARGS__))

    template<typename... Args>
    inline void assert_message([[maybe_unused]] FILE* output_stream, [[maybe_unused]] bool hard, [[maybe_unused]] const char* fmt, [[maybe_unused]] Args&&... args)
    {
#if TZ_DEBUG
        // Use the given ostream.
        fflush(output_stream);
        fprintf(output_stream, fmt, std::forward<Args>(args)...);
        ::debug_break();
        if(hard)
        {
            fprintf(output_stream, "%s", "[HARD ASSERT DETECTED. ABORTING.]\n");
            std::abort();
        }
#endif
    }

    #define tz_error9(fmt, ...) static_assert(false, "tz_error(...) invoked with 9 total arguments. 8 or more is unsupported.")
    #define tz_error2ormore(fmt, ...) tz_assert(false, fmt, __VA_ARGS__)
    #define tz_error1(msg) tz_error2ormore(msg, "")
    #define tz_error0() tz_error1("tz::error() invoked.")
    #define tz_errorX(x, A, B, C, D, E, F, G, H, I, FUNC, ...) FUNC
    #define tz_error(...) tz_errorX(,##__VA_ARGS__,\
                          tz_error9(__VA_ARGS__),\
                          tz_error2ormore(__VA_ARGS__),\
                          tz_error2ormore(__VA_ARGS__),\
                          tz_error2ormore(__VA_ARGS__),\
                          tz_error2ormore(__VA_ARGS__),\
                          tz_error2ormore(__VA_ARGS__),\
                          tz_error2ormore(__VA_ARGS__),\
                          tz_error2ormore(__VA_ARGS__),\
                          tz_error1(__VA_ARGS__),\
                          tz_error0(__VA_ARGS__)\
                          )
}

#endif // TOPAZ_CORE_ASSERT_HPP