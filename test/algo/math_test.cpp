//
// Created by Harrand on 11/04/2020.
//

#include "test_framework.hpp"
#include "algo/math.hpp"

tz::test::Case schmitt()
{
    tz::test::Case test_case("tz::algo Math Tests");
    
    int lo = 5;
    int hi = 10;
    // Typical schmitt usage. 7 is closer to 5 than 10.
    {
        int ret = tz::algo::schmitt(lo, hi, 7);
        topaz_expect(test_case, ret == lo, "schmitt(5, 10, 7) failed to return ", lo, ". Returned ", ret);
    }
    
    // Same as before. 7 is still closer to 5 than 10 so even with the higher bound we expect lo.
    {
        int ret = tz::algo::schmitt(lo, hi, 7, tz::algo::SchmittBound::Higher);
        topaz_expect(test_case, ret == lo, "schmitt(5, 10, 7, SchmittBound::Higher) failed to return ", lo, ". Returned ", ret);
    }

    // Typical schmittf usage.
    {
        int ret = tz::algo::schmittf(lo, hi, 7.5f);
        topaz_expect(test_case, ret == lo, "schmittf(5, 10, 7.5f) failed to return ", lo, ". Returned ", ret);
    }

    // Typical schmittf higher usage.
    {
        int ret = tz::algo::schmittf(lo, hi, 7.5f, tz::algo::SchmittBound::Higher);
        topaz_expect(test_case, ret == hi, "schmittf(5, 10, 7.5f, SchmittBound::Higher) failed to return ", hi, ". Returned ", ret);
    }

    return test_case;
}

int main()
{
	tz::test::Unit math;
	
    math.add(schmitt());

	return math.result();
}