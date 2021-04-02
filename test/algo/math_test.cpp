//
// Created by Harrand on 11/04/2020.
//

#include "test_framework.hpp"
#include "algo/math.hpp"

TZ_TEST_BEGIN(schmitt)
	int lo = 5;
	int hi = 10;
	// Typical schmitt usage. 7 is closer to 5 than 10.
	{
		int ret = tz::algo::schmitt(lo, hi, 7);
		topaz_expect(ret == lo, "schmitt(5, 10, 7) failed to return ", lo, ". Returned ", ret);
	}
	
	// Same as before. 7 is still closer to 5 than 10 so even with the higher bound we expect lo.
	{
		int ret = tz::algo::schmitt(lo, hi, 7, tz::algo::SchmittBound::Higher);
		topaz_expect(ret == lo, "schmitt(5, 10, 7, SchmittBound::Higher) failed to return ", lo, ". Returned ", ret);
	}

	// Typical schmittf usage.
	{
		int ret = tz::algo::schmitt(lo, hi, 7.5f);
		topaz_expect(ret == lo, "schmitt(5, 10, 7.5f) failed to return ", lo, ". Returned ", ret);
	}

	// Typical schmittf higher usage.
	{
		int ret = tz::algo::schmitt(lo, hi, 7.5f, tz::algo::SchmittBound::Higher);
		topaz_expect(ret == hi, "schmitt(5, 10, 7.5f, SchmittBound::Higher) failed to return ", hi, ". Returned ", ret);
	}
TZ_TEST_END

TZ_TEST_BEGIN(schmitt_multiple)
	{
		int ret = tz::algo::schmitt_multiple(8, 10);
		topaz_expect(ret == 8, "schmitt_multiple(8, 10) failed to return ", 8, ". Returned ", ret);
	}

	{
		int ret = tz::algo::schmitt_multiple(8, 14);
		topaz_expect(ret == 16, "schmitt_multiple(8, 14) failed to return ", 16, ". Returned ", ret);
	}

	{
		int ret = tz::algo::schmitt_multiple(5, 2.5f);
		topaz_expect(ret == 0, "schmitt_multiple(5, 2.5f) failed to return ", 0, ". Returned ", ret);
	}

	{
		int ret = tz::algo::schmitt_multiple(5, 2.5f, tz::algo::SchmittBound::Higher);
		topaz_expect(ret == 5, "schmitt_multiple(5, 2.5f, SchmittBound::Higher) failed to return ", 5, ". Returned ", ret);
	}
TZ_TEST_END

TZ_TEST_BEGIN(type_mix)
	{
		constexpr float expected = 5.0f;
		float actual = tz::algo::linear_interpolate(0, 10, 0.5f);
		topaz_expectf(expected == actual, "linear_interpolate(%d, %d, %g) == %g but resulted in %g", 0, 10, 0.5f, expected, actual);
	}

	{
		constexpr float expected = 0.0f;
		float actual = tz::algo::linear_interpolate(-5, 5u, 0.5);
		topaz_expectf(expected == actual, "linear_interpolate(%d, %du, %g) == %g but resulted in %g", -5, 5, 0.5f, expected, actual);
	}

	{
		constexpr float expected = 16.0f;
		float actual = tz::algo::schmitt(16.0f, 20, 17u);
		topaz_expectf(expected == actual, "schmitt(%d.0f, %d, %gu) == %g but resulted in %g", 16, 20, 17, expected, actual);
	}
TZ_TEST_END



int main()
{
	tz::test::Unit math;
	
	math.add(schmitt());
	math.add(schmitt_multiple());
	math.add(type_mix());

	return math.result();
}