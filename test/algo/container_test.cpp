//
// Created by Harrand on 12/04/2020.
//

#include "test_framework.hpp"
#include "algo/container.hpp"
#include "memory/pool.hpp"
#include <string>

template<typename Container>
std::string container_to_string(const Container& t)
{
	std::string result = "{";
	std::size_t size = 0;
	for(const auto& element : t)
	{
		if(size++ > 0)
		{
			result += ", ";
		}
		result += std::to_string(element);
	}
	return result + "}";
}

TZ_TEST_BEGIN(sizeof_element)
	auto three_way_comparison = [](auto a, auto b, auto c){return a == b && b == c;};

	std::vector<int> ints;
	topaz_expect(three_way_comparison(tz::algo::sizeof_element<std::vector<int>>(), tz::algo::sizeof_element(ints), sizeof(int)), "sizeof_element std::vector<int> != sizeof(int)");
	float data[1];
	tz::mem::UniformPool<float> floats{{data, sizeof(float)}};
	topaz_expect(three_way_comparison(tz::algo::sizeof_element<tz::mem::UniformPool<float>>(), tz::algo::sizeof_element(floats), sizeof(float)), "sizeof_element tz::mem::UniformPool<float> != sizeof(float)");
TZ_TEST_END

TZ_TEST_BEGIN(container_contains)
	std::vector<int> ints{0, 1, 2, 3};
	topaz_expectf(tz::algo::contains_element(ints, 0), "%s wrongly thinks it doesn't contain the element %d", container_to_string(ints).c_str(), 0);
	topaz_expectf(!tz::algo::contains_element(ints, 14), "%s wrongly thinks it contains the element %d", container_to_string(ints).c_str(), 14);
TZ_TEST_END

int main()
{
	tz::test::Unit math;
	
	math.add(sizeof_element());
	math.add(container_contains());

	return math.result();
}