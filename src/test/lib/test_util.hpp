#ifndef TOPAZ_TEST_UTIL_HPP
#define TOPAZ_TEST_UTIL_HPP
#include "test_failure_exception.hpp"

namespace tz::assert
{
    inline void that(bool assertion)
    {
        if(!assertion)
            throw TestFailureException("tz::assert::that(bool) assertion failed.");
    }

    template<typename X, typename Y>
    inline void equal(const X& a, const Y& b)
    {
        using namespace tz::utility::generic::cast;
        if(!(a == b))
            throw TestFailureException("tz::assert::equal(a, b) assertion failed: lhs = " + to_string(a) + ", rhs = " + to_string(b));
    }

    template<typename X, typename Y>
    inline void inequal(const X& a, const Y& b)
    {
        using namespace tz::utility::generic::cast;
        if(a == b)
            throw TestFailureException("tz::assert::inequal(a, b) assertion failed: lhs = " + to_string(a) + ", rhs = " + to_string(b));
    }
}

#endif //TOPAZ_TEST_UTIL_HPP