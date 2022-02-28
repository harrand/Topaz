//
// Created by Harrand on 06/01/2020.
//

#include "core/tz.hpp"
#include "core/vector.hpp"
#include <string>
#include <cstring>

void addition_subtraction()
{
	tz::Vec4 a(1.0f, 2.0f, 3.0f, 4.0f);
	tz::Vec4 b(8.0f, 7.0f, 6.0f, 5.0f);

	// Addition
	{
		const tz::Vec4 d = a + b;
		tz_assert(d[0] == 9.0f, "tz::geo::Vec4 Addition yielded unexpected value. Expected %g, but got %g", 9, d[0]);
		tz_assert(d[1] == 9.0f, "tz::geo::Vec4 Addition yielded unexpected value. Expected %g, but got %g", 9, d[1]);
		tz_assert(d[2] == 9.0f, "tz::geo::Vec4 Addition yielded unexpected value. Expected %g, but got %g", 9, d[2]);
		tz_assert(d[3] == 9.0f, "tz::geo::Vec4 Addition yielded unexpected value. Expected %g, but got %g", 9, d[3]);
	}

	// Subtraction
	{
		const tz::Vec4 d = b - a;
		constexpr tz::Vec4 e{{7.0f, 5.0f, 3.0f, 1.0f}};
		for(std::size_t i = 0; i < 4; i++)
		{
			tz_assert(d[i] == e[i], "tz::geo::Vec4 Subtraction yielded unexpected value. Expected %g, but got %g", e[i], d[i]);
		}
	}
}

void dot()
{
	tz::Vec4 a{18.0f, 39.0f, 31.0f, 1.0f};
	tz::Vec4 b{101.0f, 58.0f, 75.0f, 1.0f};
	tz::Vec4 c{9.0f, 2.0f, 7.0f, 1.0f};
	tz::Vec4 d{4.0f, 8.0f, 10.0f, 1.0f};

	tz_assert(a.length() == 52.981128f, "tz::gl::Vec4 Produced incorrect magnitude of: ", a.length());
	tz_assert(a.dot(b) == 6406, "tz::gl::Vec4 Dot Product yielded incorrect result. Expected %g, but got %g", 6406.0f, a.dot(b));
	tz_assert(c.dot(d) == 123.0f, "tz::gl::Vec4 Dot Product yielded incorrect result. Expected %g, but got %g", 123.0f, c.dot(d));
}

void cross()
{
	tz::Vec3 a(2.0f, 3.0f, 4.0f);
	tz::Vec3 b(5.0f, 6.0f, 7.0f);
	tz::Vec3 c = tz::cross(a, b);
	tz_assert(c[0] == -3, "Cross Product incorrect.");
	tz_assert(c[1] == 6, "Cross Product incorrect.");
	tz_assert(c[2] == -3, "Cross Product incorrect.");
	// (2, 3, 4) × (5, 6, 7) = (−3, 6, −3)
}

void swizzle_test()
{
	tz::Vec4 a{1.0f, 2.0f, 3.0f, 4.0f};
	tz::Vec4 same = a.swizzle<0, 1, 2, 3>();
	tz::Vec4 backward = a.swizzle<3, 2, 1, 0>();
	for(std::size_t i = 0; i < 4; i++)
	{
		tz_assert(a[i] == same[i], "Swizzle xyzw failed (idx = %zu, before = %g, after = %g)", i, a[i], same[i]);
		tz_assert(a[i] == backward[3-i], "Swizzle wzyx failed (idx = %zu, before = %g, after = %g)", i, a[i], backward[3-i]);
	}

	tz::Vec3 smaller = a.swizzle<0, 1, 2>();
	tz_assert(smaller == tz::Vec3(1.0f, 2.0f, 3.0f), "Swizzle from Vec4 to Vec3 via xyz failed.");
	tz::Vec4 single = a.swizzle<0, 0, 0, 0>();
	tz_assert(single == tz::Vec4(1.0f, 1.0f, 1.0f, 1.0f), "Swizzle using multiple of the same indices failed.");
}

int main()
{
	addition_subtraction();
	dot();
	cross();
	swizzle_test();
}
