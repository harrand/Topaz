//
// Created by Harry on 30/12/2018.
//

#define UNIT_TEST "Numeric"
#include "lib/test_util.hpp"
#include "utility/numeric.hpp"

void test()
{
    using namespace tz::utility;
    tz::assert::equal(numeric::linear_interpolate(0.0f, 1.0f, 0.5f), 0.5f);
    tz::assert::inequal(numeric::cosine_interpolate(0.0f, 1.0f, 0.5f), 0.5f);

    std::vector<float> values{1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    tz::assert::equal(numeric::variance(values), 2.5f);
    tz::assert::equal(numeric::standard_deviation(values), std::sqrt(2.5f));
}
