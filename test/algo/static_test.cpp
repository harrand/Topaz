//
// Created by Harrand on 23/04/2020.
//

#include "test_framework.hpp"
#include "algo/static.hpp"
#include <array>

template<typename T>
void handler(std::size_t& i, [[maybe_unused]] T t)
{
    auto handle_char = [&i]([[maybe_unused]] char c){i++;};
    auto handle_int = [&i]([[maybe_unused]] int i){i *= 2;};
    auto handle_float = [&i]([[maybe_unused]] float f){i *= i;};
    if constexpr(std::is_same_v<T, char>)
    {
        handle_char(0);
    }
    else if constexpr(std::is_same_v<T, int>)
    {
        handle_int(0);
    }
    else if constexpr(std::is_same_v<T, float>)
    {
        handle_float(0.0f);
    }
}

tz::test::Case static_for()
{
    tz::test::Case test_case("tz::algo Static For Tests");
    std::size_t i = 0;
    tz::algo::static_for<0, 100>([&i](auto idx){i += idx;});
    std::size_t actual = 0;
    for(int i = 0; i < 100; i++)
    {
        actual += i;
    }

    topaz_expect(test_case, actual == i, "Uh oh");
    return test_case;
}

tz::test::Case static_parameter_expansion()
{
    tz::test::Case test_case("tz::algo Static Parameter Expansion Tests");

    float f;
    int i;
    char c;
    tz::algo::static_for<0, 3>([&f, &i, &c]([[maybe_unused]] auto idx) constexpr{
        switch(idx)
        {
            case 0:
                f = 0.0f;
            break;
            case 1:
                i = 1;
            break;
            case 2:
                c = '2';
            break;
        }
    });
    topaz_expect(test_case, f == 0.0f, "Fail");
    topaz_expect(test_case, i == 1, "Fail");
    topaz_expect(test_case, c == '2', "Fail");

    return test_case;
}

int main()
{
	tz::test::Unit statics;
	
    statics.add(static_for());
    statics.add(static_parameter_expansion());

	return statics.result();
}