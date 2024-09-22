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

void test_basic_operations()
{
	// Test indexing operator (const and non-const)
    tz::v3f vec1(1.0f, 2.0f, 3.0f);
    tz_assert(vec1[0] == 1.0f, "Indexing failed. Expected 1.0f, got %f", vec1[0]);
    tz_assert(vec1[1] == 2.0f, "Indexing failed. Expected 2.0f, got %f", vec1[1]);
    tz_assert(vec1[2] == 3.0f, "Indexing failed. Expected 3.0f, got %f", vec1[2]);

    // Modify using non-const indexing
    vec1[0] = 10.0f;
    tz_assert(vec1[0] == 10.0f, "Indexing failed after modification. Expected 10.0f, got %f", vec1[0]);
	// Test += operator
    tz::v3f vec2(1.0f, 1.0f, 1.0f);
    vec1 += vec2;
    tz_assert((vec1 == tz::v3f(11.0f, 3.0f, 4.0f)), 
              "Addition assignment failed. Expected: {{11.0f, 3.0f, 4.0f}}, got: {{{}, {}, {}}}", vec1[0], vec1[1], vec1[2]);

    // Test -= operator
    vec1 -= vec2;
    tz_assert((vec1 == tz::v3f(10.0f, 2.0f, 3.0f)), 
              "Subtraction assignment failed. Expected: {{10.0f, 2.0f, 3.0f}}, got: {{{}, {}, {}}}", vec1[0], vec1[1], vec1[2]);

    // Test *= operator
    vec1 *= vec2;
    tz_assert((vec1 == tz::v3f(10.0f, 2.0f, 3.0f)), 
              "Multiplication assignment failed. Expected: {{10.0f, 2.0f, 3.0f}}, got: {{{}, {}, {}}}", vec1[0], vec1[1], vec1[2]);

    // Test /= operator
    tz::v3f vec3(10.0f, 1.0f, 3.0f);
    vec3 /= tz::v3f(1.0f, 1.0f, 1.0f);  // Divide each element by 1
    tz_assert((vec3 == tz::v3f(10.0f, 1.0f, 3.0f)),
              "Division assignment failed. Expected: {{10.0f, 1.0f, 3.0f}}, got: {{{}, {}, {}}}", vec3[0], vec3[1], vec3[2]);

    // Test + operator
    auto sum_vec = vec1 + vec2;
    tz_assert((sum_vec == tz::v3f(11.0f, 3.0f, 4.0f)), 
              "Addition failed. Expected: {{11.0f, 3.0f, 4.0f}}, got: {{{}, {}, {}}}", sum_vec[0], sum_vec[1], sum_vec[2]);

    // Test - operator
    auto diff_vec = vec1 - vec2;
    tz_assert((diff_vec == tz::v3f(9.0f, 1.0f, 2.0f)), 
              "Subtraction failed. Expected: {{9.0f, 1.0f, 2.0f}}, got: {{{}, {}, {}}}", diff_vec[0], diff_vec[1], diff_vec[2]);

    // Test * operator
    auto prod_vec = vec1 * vec2;
    tz_assert((prod_vec == tz::v3f(10.0f, 2.0f, 3.0f)),
              "Multiplication failed. Expected: {{10.0f, 2.0f, 3.0f}}, got: {{{}, {}, {}}}", prod_vec[0], prod_vec[1], prod_vec[2]);

    // Test / operator
    auto div_vec = vec1 / tz::v3f(1.0f, 1.0f, 1.0f);
    tz_assert((div_vec == tz::v3f(10.0f, 2.0f, 3.0f)),
              "Division failed. Expected: {{10.0f, 2.0f, 3.0f}}, got: {{{}, {}, {}}}", div_vec[0], div_vec[1], div_vec[2]);
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
	test_basic_operations();
	return 0;
}
