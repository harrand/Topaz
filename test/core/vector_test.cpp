#include "tz/core/tz_core.hpp"
#include "tz/core/debug.hpp"
#include "tz/core/data/vector.hpp"
#include <string>
#include <cstring>

void addition_subtraction()
{
	tz::vec4 a(1.0f, 2.0f, 3.0f, 4.0f);
	tz::vec4 b(8.0f, 7.0f, 6.0f, 5.0f);

	// Addition
	{
		const tz::vec4 d = a + b;
		tz::assert(d[0] == 9.0f, "tz::geo::vec4 Addition yielded unexpected value. Expected %g, but got %g", 9, d[0]);
		tz::assert(d[1] == 9.0f, "tz::geo::vec4 Addition yielded unexpected value. Expected %g, but got %g", 9, d[1]);
		tz::assert(d[2] == 9.0f, "tz::geo::vec4 Addition yielded unexpected value. Expected %g, but got %g", 9, d[2]);
		tz::assert(d[3] == 9.0f, "tz::geo::vec4 Addition yielded unexpected value. Expected %g, but got %g", 9, d[3]);
	}

	// Subtraction
	{
		const tz::vec4 d = b - a;
		constexpr tz::vec4 e{{7.0f, 5.0f, 3.0f, 1.0f}};
		for(std::size_t i = 0; i < 4; i++)
		{
			tz::assert(d[i] == e[i], "tz::geo::vec4 Subtraction yielded unexpected value. Expected %g, but got %g", e[i], d[i]);
		}
	}
}

void dot()
{
	tz::vec4 a{18.0f, 39.0f, 31.0f, 1.0f};
	tz::vec4 b{101.0f, 58.0f, 75.0f, 1.0f};
	tz::vec4 c{9.0f, 2.0f, 7.0f, 1.0f};
	tz::vec4 d{4.0f, 8.0f, 10.0f, 1.0f};

	tz::assert(a.length() == 52.981128f, "tz::gl::vec4 Produced incorrect magnitude of: ", a.length());
	tz::assert(a.dot(b) == 6406, "tz::gl::vec4 Dot Product yielded incorrect result. Expected %g, but got %g", 6406.0f, a.dot(b));
	tz::assert(c.dot(d) == 123.0f, "tz::gl::vec4 Dot Product yielded incorrect result. Expected %g, but got %g", 123.0f, c.dot(d));
}

void cross()
{
	tz::vec3 a(2.0f, 3.0f, 4.0f);
	tz::vec3 b(5.0f, 6.0f, 7.0f);
	tz::vec3 c = tz::cross(a, b);
	tz::assert(c[0] == -3, "Cross Product incorrect.");
	tz::assert(c[1] == 6, "Cross Product incorrect.");
	tz::assert(c[2] == -3, "Cross Product incorrect.");
	// (2, 3, 4) × (5, 6, 7) = (−3, 6, −3)
}

void swizzle_test()
{
	tz::vec4 a{1.0f, 2.0f, 3.0f, 4.0f};
	tz::vec4 same = a.swizzle<0, 1, 2, 3>();
	tz::vec4 backward = a.swizzle<3, 2, 1, 0>();
	for(std::size_t i = 0; i < 4; i++)
	{
		tz::assert(a[i] == same[i], "Swizzle xyzw failed (idx = %zu, before = %g, after = %g)", i, a[i], same[i]);
		tz::assert(a[i] == backward[3-i], "Swizzle wzyx failed (idx = %zu, before = %g, after = %g)", i, a[i], backward[3-i]);
	}

	tz::vec3 smaller = a.swizzle<0, 1, 2>();
	tz::assert(smaller == tz::vec3(1.0f, 2.0f, 3.0f), "Swizzle from vec4 to vec3 via xyz failed.");
	tz::vec4 single = a.swizzle<0, 0, 0, 0>();
	tz::assert(single == tz::vec4(1.0f, 1.0f, 1.0f, 1.0f), "Swizzle using multiple of the same indices failed.");
}

void cast_test()
{
	tz::vec2ui two_unsigneds{1u, 2u};
	auto signeds = static_cast<tz::vec2i>(two_unsigneds);
	tz::assert(signeds == tz::vec2i(1, 2), "vector cast between different numeric types yielded incorrect values. Expected {%d, %d} but got {%d, %d}", 1, 2, signeds[0], signeds[1]);
}

void append_tests()
{
	tz::vec2ui nums{1u, 2u};
	tz::vec3ui more_nums = nums.with_more(3u);

	constexpr tz::vec3ui result0{1u, 2u, 3u};
	tz::assert(more_nums == result0, "vector append (single T) does not work correctly.");
	tz::vec4ui even_more_nums = nums.with_more(nums);
	constexpr tz::vec4ui result1{1u, 2u, 1u, 2u};
	tz::assert(even_more_nums == result1, "vector append (another vector) does not work correctly.");
}

void zero()
{
	tz::assert(tz::vec2i::filled(5) == tz::vec2i{5, 5});
	tz::assert(tz::vec2i::zero() == tz::vec2i{0, 0});
}

int main()
{
	addition_subtraction();
	dot();
	cross();
	swizzle_test();
	cast_test();
	append_tests();
	zero();
}
