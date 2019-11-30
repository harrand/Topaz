//
// Created by Harrand on 25/08/2019.
//

#ifndef TOPAZ_ASSERT_HPP
#define TOPAZ_ASSERT_HPP
#include <iostream>

namespace tz
{
#ifdef TOPAZ_DEBUG
    constexpr bool is_debug_mode = true;
#else
    constexpr bool is_debug_mode = false;
#endif

#ifdef topaz_assert
#undef topaz_assert
#endif
#define topaz_assert(EXPRESSION, ...) ((EXPRESSION) ? \
(void)0 : tz::assert_message(std::cerr, \
"Assertion failure: ", #EXPRESSION, "\nIn file: ", __FILE__, \
" on line ", __LINE__, ":\n\t", __VA_ARGS__))

    template<typename... Args>
    inline void assert_message(std::ostream &out, Args &&... args)
    {
        if constexpr(tz::is_debug_mode)
        {
            std::flush(out);
            (out << ... << args) << std::endl;
            std::abort();
        }
    }
}

#endif //TOPAZ_ASSERT_HPP
