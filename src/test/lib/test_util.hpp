#ifndef TOPAZ_TEST_UTIL_HPP
#define TOPAZ_TEST_UTIL_HPP
#include "test_failure_exception.hpp"
#include "core/topaz.hpp"
#include "utility/time.hpp"
#include <iostream>

namespace tz::unit_test
{
    decltype(tz::utility::time::now()) before;
    inline void begin_unit_test()
    {
        before = tz::utility::time::now();
    }

    inline void end_unit_test()
    {
        auto tdiff = tz::utility::time::now() - before;
        std::cout << "Time taken = " << tdiff << "ms\n";
    }
}

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

#ifndef UNIT_TEST
#define UNIT_TEST "Unknown"
#endif

void test();

int main()
{
    tz::initialise();
    tz::unit_test::begin_unit_test();
    std::cout << UNIT_TEST << " Test Results: ";
    try
    {
        test();
        std::cout << "PASS\n";
    }catch(const TestFailureException& fail)
    {
        std::cerr << "FAIL: " << fail.what() << "\n";
    }
    tz::unit_test::end_unit_test();
    tz::terminate();
    return 0;
}

#endif //TOPAZ_TEST_UTIL_HPP