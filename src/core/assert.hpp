#ifndef TOPAZ_CORE_ASSERT_HPP
#define TOPAZ_CORE_ASSERT_HPP
#include "debugbreak.h"
#include <cstdio>
#include <cstdlib>
#include <utility>

namespace tz
{
    template<typename... Args>
    void error_internal(const char* fmt, Args&&... args)
    {
        #if TZ_DEBUG
            std::fflush(stderr);
            std::fprintf(stderr, fmt, std::forward<Args>(args)...);
            std::fflush(stderr);
            ::debug_break();
        #endif
    }

    template<typename... Args>
    void assert_internal(bool eval, const char* fmt, Args&&... args)
    {
        #if TZ_DEBUG
            if(!eval)
            {
                std::fflush(stderr);
                std::fprintf(stderr, fmt, std::forward<Args>(args)...);
                std::fflush(stderr);
                ::debug_break();
            }
        #endif
    }
}
#define tz_error(msg, ...) {tz::error_internal("tz_error:\nIn file %s:%d:\n\t " msg "\n", __FILE__, __LINE__, ##__VA_ARGS__);} (void)0;
#define tz_assert(eval, msg, ...) {tz::assert_internal(eval, "tz_assert Failure: %s\nIn file: %s:%d:\n\t " msg "\n", #eval, __FILE__, __LINE__, ##__VA_ARGS__);} (void)0;

#endif // TOPAZ_CORE_ASSERT_HPP