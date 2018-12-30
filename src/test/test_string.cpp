//
// Created by Harry on 30/12/2018.
//

#define UNIT_TEST "String"
#include "lib/test_util.hpp"
#include "utility/string.hpp"

void test()
{
    using namespace tz::utility;
    tz::assert::equal(string::to_lower("HeLLO THErE"), std::string{"hello there"});
    tz::assert::equal(string::to_upper("helLo THere"), std::string{"HELLO THERE"});

    tz::assert::that(string::begins_with("greetings", "gree") && string::begins_with("well met", "w"));
    tz::assert::that(string::ends_with("people", "ople") && string::ends_with("death to the horde", "e"));
    tz::assert::that(string::contains("slaughter", 'e') && !string::contains("slaughter", 'z'));
    auto split = string::split_string("hello there i am andrew thorne", " ");
    tz::assert::equal(split.size(), 6ull);
    tz::assert::equal(split[0], std::string{"hello"});
    tz::assert::equal(split[1], std::string{"there"});
    tz::assert::equal(split[2], std::string{"i"});
    tz::assert::equal(split[3], std::string{"am"});
    tz::assert::equal(split[4], std::string{"andrew"});
    tz::assert::equal(split[5], std::string{"thorne"});

    tz::assert::equal(string::replace_all("hello there i am yellow-friend", "l", "r"), std::string{"herro there i am yerrow-friend"});
    tz::assert::equal(string::substring("japanese", 0, 4), std::string{"japan"});
}