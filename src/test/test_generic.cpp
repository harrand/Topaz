//
// Created by Harry on 30/12/2018.
//

#define UNIT_TEST "Generic"
#include "lib/test_util.hpp"

void test_sizeof_container()
{
    std::vector<int> ints;
    std::vector<std::string> strings;
    using namespace tz::utility::generic;
    tz::assert::equal(sizeof(int), sizeof_element(ints));
    tz::assert::equal(sizeof(std::string), sizeof_element(strings));
}

void test_contains()
{
    std::vector<std::string> strings;
    using namespace tz::utility::generic;
    strings.push_back("hello there");
    tz::assert::that(contains(strings, std::string{"hello there"}));
}

void test_cast_to()
{
    float x = 1.024f;
    tz::assert::equal<std::string>("1.024", tz::utility::generic::cast::to_string(x));
}

void test_cast_from()
{
    float x = tz::utility::generic::cast::from_string<float>("2.048");
    tz::assert::equal(2.048f, x);
}

void test()
{
    test_sizeof_container();
    test_contains();
    test_cast_to();
    test_cast_from();
}