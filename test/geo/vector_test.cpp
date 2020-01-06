//
// Created by Harrand on 06/01/2020.
//

#include "test_framework.hpp"
#include "core/core.hpp"
#include "geo/vector.hpp"
#include <string>
#include <cstring>

tz::test::Case simple_handle()
{
    tz::test::Case test_case("tz::geo::sse Simple Handle Test");
    tz::geo::sse::SSEValidArray<float, 4> arr{4.0f, 16.0f, 36.0f, 4.0f};
    auto* handle = tz::geo::sse::handle_array(arr);
    auto result_handle = _mm_sqrt_ps(*handle);
    tz::geo::sse::SSEValidArray<float, 4> res;
    _mm_store_ps(res, result_handle);
    topaz_expect(test_case, res[0] == 2.0f, "fuck ", res[0]);
    topaz_expect(test_case, res[1] == 4.0f, "fuck1 ", res[1]);
    topaz_expect(test_case, res[2] == 6.0f, "fuck2 ", res[2]);
    return test_case;
}

int main()
{
    tz::test::Unit vec;

    // Stuff in here requires topaz to be initialised.
    {
        tz::core::initialise("Vector Tests");
        vec.add(simple_handle());
        tz::core::terminate();
    }

    return vec.result();
}