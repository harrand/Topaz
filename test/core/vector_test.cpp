//
// Created by Harrand on 06/01/2020.
//

#include "tz/core/tz.hpp"
#include "tz/core/vector.hpp"
#include <string>
#include <cstring>

void addition_subtraction()
{
	tz::Vec4 a(1.0f, 2.0f, 3.0f, 4.0f);
	tz::Vec4 b(8.0f, 7.0f, 6.0f, 5.0f);

	// Addition
	{
		const tz::Vec4 d = a + b;
		hdk::assert(d[0] == 9.0f, "tz::geo::Vec4 Addition yielded unexpected value. Expected %g, but got %g", 9, d[0]);
		hdk::assert(d[1] == 9.0f, "tz::geo::Vec4 Addition yielded unexpected value. Expected %g, but got %g", 9, d[1]);
		hdk::assert(d[2] == 9.0f, "tz::geo::Vec4 Addition yielded unexpected value. Expected %g, but got %g", 9, d[2]);
		hdk::assert(d[3] == 9.0f, "tz::geo::Vec4 Addition yielded unexpected value. Expected %g, but got %g", 9, d[3]);
	}

	// Subtraction
	{
		const tz::Vec4 d = b - a;
		constexpr tz::Vec4 e{{7.0f, 5.0f, 3.0f, 1.0f}};
		for(std::size_t i = 0; i < 4; i++)
		{
			hdk::assert(d[i] == e[i], "tz::geo::Vec4 Subtraction yielded unexpected value. Expected %g, but got %g", e[i], d[i]);
		}
	}
}

void dot()
{
	tz::Vec4 a{18.0f, 39.0f, 31.0f, 1.0f};
	tz::Vec4 b{101.0f, 58.0f, 75.0f, 1.0f};
	tz::Vec4 c{9.0f, 2.0f, 7.0f, 1.0f};
	tz::Vec4 d{4.0f, 8.0f, 10.0f, 1.0f};

	hdk::assert(a.length() == 52.981128f, "tz::gl::Vec4 Produced incorrect magnitude of: ", a.length());
	hdk::assert(a.dot(b) == 6406, "tz::gl::Vec4 Dot Product yielded incorrect result. Expected %g, but got %g", 6406.0f, a.dot(b));
	hdk::assert(c.dot(d) == 123.0f, "tz::gl::Vec4 Dot Product yielded incorrect result. Expected %g, but got %g", 123.0f, c.dot(d));
}

void cross()
{
	tz::Vec3 a(2.0f, 3.0f, 4.0f);
	tz::Vec3 b(5.0f, 6.0f, 7.0f);
	tz::Vec3 c = tz::cross(a, b);
	hdk::assert(c[0] == -3, "Cross Product incorrect.");
	hdk::assert(c[1] == 6, "Cross Product incorrect.");
	hdk::assert(c[2] == -3, "Cross Product incorrect.");
	// (2, 3, 4) × (5, 6, 7) = (−3, 6, −3)
}

void swizzle_test()
{
	tz::Vec4 a{1.0f, 2.0f, 3.0f, 4.0f};
	tz::Vec4 same = a.swizzle<0, 1, 2, 3>();
	tz::Vec4 backward = a.swizzle<3, 2, 1, 0>();
	for(std::size_t i = 0; i < 4; i++)
	{
		hdk::assert(a[i] == same[i], "Swizzle xyzw failed (idx = %zu, before = %g, after = %g)", i, a[i], same[i]);
		hdk::assert(a[i] == backward[3-i], "Swizzle wzyx failed (idx = %zu, before = %g, after = %g)", i, a[i], backward[3-i]);
	}

	tz::Vec3 smaller = a.swizzle<0, 1, 2>();
	hdk::assert(smaller == tz::Vec3(1.0f, 2.0f, 3.0f), "Swizzle from Vec4 to Vec3 via xyz failed.");
	tz::Vec4 single = a.swizzle<0, 0, 0, 0>();
	hdk::assert(single == tz::Vec4(1.0f, 1.0f, 1.0f, 1.0f), "Swizzle using multiple of the same indices failed.");
}

void cast_test()
{
	tz::Vec2ui two_unsigneds{1u, 2u};
	auto signeds = static_cast<tz::Vec2i>(two_unsigneds);
	hdk::assert(signeds == tz::Vec2i(1, 2), "Vector cast between different numeric types yielded incorrect values. Expected {%d, %d} but got {%d, %d}", 1, 2, signeds[0], signeds[1]);
}

void append_tests()
{
	tz::Vec2ui nums{1u, 2u};
	tz::Vec3ui more_nums = nums.with_more(3u);

	constexpr tz::Vec3ui result0{1u, 2u, 3u};
	hdk::assert(more_nums == result0, "Vector append (single T) does not work correctly.");
	tz::Vec4ui even_more_nums = nums.with_more(nums);
	constexpr tz::Vec4ui result1{1u, 2u, 1u, 2u};
	hdk::assert(even_more_nums == result1, "Vector append (another Vector) does not work correctly.");
}

int main()
{
	addition_subtraction();
	dot();
	cross();
	swizzle_test();
	cast_test();
	append_tests();
}
