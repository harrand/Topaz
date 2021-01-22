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
	auto handle_int = [&i]([[maybe_unused]] int ii){i *= 2;};
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

TZ_TEST_BEGIN(static_for)
	std::size_t i = 0;
	tz::algo::static_for<0, 100>([&i](auto idx){i += idx;});
	std::size_t actual = 0;
	for(int i = 0; i < 100; i++)
	{
		actual += i;
	}

	topaz_expect(actual == i, "Uh oh");
TZ_TEST_END

TZ_TEST_BEGIN(static_parameter_expansion)
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
	topaz_expect(f == 0.0f, "Fail");
	topaz_expect(i == 1, "Fail");
	topaz_expect(c == '2', "Fail");
TZ_TEST_END

int main()
{
	tz::test::Unit statics;
	
	statics.add(static_for());
	statics.add(static_parameter_expansion());

	return statics.result();
}