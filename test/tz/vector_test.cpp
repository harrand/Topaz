#include "tz/topaz.hpp"
#include "tz/core/vector.hpp"
#include <numeric>

template<typename T, int N>
void test_constructor()
{
	using vec_t = tz::vector<T, N>;
	// Test default constructor
    vec_t vec;  // Default constructor - the values are indeterminate.

    // Test filled constructor
    auto filled_vec = vec_t::filled(static_cast<T>(1.5f));  // Should fill with 1.5f
	std::array<T, N> arr0;
	std::fill(arr0.begin(), arr0.end(), static_cast<T>(1.5f));
	tz_assert(filled_vec == vec_t(arr0), "filled() failed. Expected: {{1.5f...}}");

    // Test zero vector creation
	std::fill(arr0.begin(), arr0.end(), static_cast<T>(0));
    auto zero_vec = vec_t::zero();  // Should return a zero vector
	tz_assert(zero_vec == vec_t(arr0), "zero() failed. Expected: {{0.0f, ...}}");

    // Test constructor with variadic arguments
    vec_t variadic_vec(static_cast<T>(1.5f));
    tz_assert((variadic_vec == vec_t{static_cast<T>(1.5f)}),
              "variadic constructor failed. Expected: {{1.5f}}");

    // Test array constructor
	std::array<T, N> arr;
	std::iota(arr.begin(), arr.end(), T{0});
    vec_t array_vec(arr);
    tz_assert((array_vec == vec_t(arr)),
              "array constructor failed. Expected: {{1.0f, 2.0f, ...}}");
}

void test_accessors()
{
	tz::v4i ints{1, 2, 3, 4};
	tz_assert(ints[0] == 1, "vector accessor failed. Expected {}, got {}", 1, ints[0]);
	tz_assert(ints[1] == 2, "vector accessor failed. Expected {}, got {}", 2, ints[1]);
	tz_assert(ints[2] == 3, "vector accessor failed. Expected {}, got {}", 3, ints[2]);
	tz_assert(ints[3] == 4, "vector accessor failed. Expected {}, got {}", 4, ints[3]);

	ints[0] = 123;
	tz_assert(ints[0] == 123, "vector accessor failed. Expected {}, got {}", 123, ints[0]);
	tz_assert(ints[1] == 2, "vector accessor failed. Expected {}, got {}", 2, ints[1]);
	tz_assert(ints[2] == 3, "vector accessor failed. Expected {}, got {}", 3, ints[2]);
	tz_assert(ints[3] == 4, "vector accessor failed. Expected {}, got {}", 4, ints[3]);
}

int main()
{
	test_constructor<int, 2>();
	test_constructor<int, 3>();
	test_constructor<int, 4>();

	test_constructor<unsigned int, 2>();
	test_constructor<unsigned int, 3>();
	test_constructor<unsigned int, 4>();

	test_constructor<float, 2>();
	test_constructor<float, 3>();
	test_constructor<float, 4>();

	test_constructor<double, 2>();
	test_constructor<double, 3>();
	test_constructor<double, 4>();
	test_accessors();
	return 0;
}
