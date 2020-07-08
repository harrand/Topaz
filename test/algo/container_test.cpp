//
// Created by Harrand on 12/04/2020.
//

#include "test_framework.hpp"
#include "algo/container.hpp"
#include "memory/pool.hpp"


tz::test::Case sizeof_element()
{
	tz::test::Case test_case("tz::algo Container Sizeof Element Tests");
	auto three_way_comparison = [](auto a, auto b, auto c){return a == b && b == c;};

	std::vector<int> ints;
	topaz_expect(test_case, three_way_comparison(tz::algo::sizeof_element<std::vector<int>>(), tz::algo::sizeof_element(ints), sizeof(int)), "sizeof_element std::vector<int> != sizeof(int)");
	float data[1];
	tz::mem::UniformPool<float> floats{{data, sizeof(float)}};
	topaz_expect(test_case, three_way_comparison(tz::algo::sizeof_element<tz::mem::UniformPool<float>>(), tz::algo::sizeof_element(floats), sizeof(float)), "sizeof_element tz::mem::UniformPool<float> != sizeof(float)");
	return test_case;
}

int main()
{
	tz::test::Unit math;
	
	math.add(sizeof_element());

	return math.result();
}