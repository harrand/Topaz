//
// Created by Harrand on 25/06/2018.
//

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

    inline void equal(long double a, long double b)
    {
        if(a != b)
            throw TestFailureException("tz::assert::equal(a, b) assertion failed: rhs = " + std::to_string(a) + ", lhs = " + std::to_string(b));
    }
}

#endif //TOPAZ_TEST_UTIL_HPP
