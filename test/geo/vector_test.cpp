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
    tz::test::Case test_case("tz::geo::sse Simple Handle Tests");
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

tz::test::Case addition_subtraction()
{
    tz::test::Case test_case("tz::geo::Vec4 SSE Addition/Subtraction Tests");
    tz::geo::Vec4 a{1.0f, 2.0f, 3.0f, 4.0f};
    tz::geo::Vec4 b{8.0f, 7.0f, 6.0f, 5.0f};

    // Addition
    {
        const tz::geo::Vec4 r = a + b;
        tz::geo::Vec4Data d = r.get();
        topaz_expect(test_case, d[0] == 9.0f, "tz::geo::Vec4 Addition yielded unexpected value. Expected ", 9, ", but got ", d[0]);
        topaz_expect(test_case, d[1] == 9.0f, "tz::geo::Vec4 Addition yielded unexpected value. Expected ", 9, ", but got ", d[1]);
        topaz_expect(test_case, d[2] == 9.0f, "tz::geo::Vec4 Addition yielded unexpected value. Expected ", 9, ", but got ", d[2]);
        topaz_expect(test_case, d[3] == 9.0f, "tz::geo::Vec4 Addition yielded unexpected value. Expected ", 9, ", but got ", d[3]);
    }

    // Subtraction
    {
        const tz::geo::Vec4 r = b - a;
        tz::geo::Vec4Data d = r.get();
        constexpr tz::geo::Vec4Data e{{7.0f, 5.0f, 3.0f, 1.0f}};
        for(std::size_t i = 0; i < 4; i++)
            topaz_expect(test_case, d[i] == e[i], "tz::geo::Vec4 Subtraction yielded unexpected value. Expected ", e[i], ", but got ", d[i]);
    }
    return test_case;
}

tz::test::Case dot()
{
    tz::test::Case test_case("tz::geo::Vec4 SSE Dot/Cross Product Tests");

    tz::geo::Vec4 a(18, 39, 31, 1.0f);
    tz::geo::Vec4 b(101, 58, 75, 1.0f);
    tz::geo::Vec4 c(9, 2, 7, 1.0f);
    tz::geo::Vec4 d(4, 8, 10, 1.0f);

    topaz_expect(test_case, a.length() == 52.981128f, "tz::gl::Vec4 Produced incorrect magnitude of: ", a.length());
    topaz_expect(test_case, a.dot(b) == 6406, "tz::gl::Vec4 Dot Product yielded incorrect result. Expected ", 6406.0f, ", but got ", a.dot(b));
    topaz_expect(test_case, c.dot(d) == 123.0f, "tz::gl::Vec4 Dot Product yielded incorrect result. Expected ", 123.0f, ", but got ", c.dot(d));

    return test_case;
}

int main()
{
    tz::test::Unit vec;

    vec.add(simple_handle());
    vec.add(addition_subtraction());
    vec.add(dot());

    return vec.result();
}