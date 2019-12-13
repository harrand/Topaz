#ifndef ASSERT_HPP
#define ASSERT_HPP
#include <iostream>

namespace tz
{

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
#if TOPAZ_DEBUG
        std::flush(out);
        (out << ... << args) << std::endl;
        std::abort();
#endif
    }
}

#endif // ASSERT_HPP