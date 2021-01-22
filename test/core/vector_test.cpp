//
// Created by Harrand on 06/01/2020.
//

#include "test_framework.hpp"
#include "core/core.hpp"
#include "core/vector.hpp"
#include <string>
#include <cstring>

tz::test::Case addition_subtraction()
{
	tz::test::Case test_case("tz::Vec4 Addition/Subtraction Tests");
	tz::Vec4 a(1.0f, 2.0f, 3.0f, 4.0f);
	tz::Vec4 b(8.0f, 7.0f, 6.0f, 5.0f);

	// Addition
	{
		const tz::Vec4 d = a + b;
		topaz_expect(test_case, d[0] == 9.0f, "tz::geo::Vec4 Addition yielded unexpected value. Expected ", 9, ", but got ", d[0]);
		topaz_expect(test_case, d[1] == 9.0f, "tz::geo::Vec4 Addition yielded unexpected value. Expected ", 9, ", but got ", d[1]);
		topaz_expect(test_case, d[2] == 9.0f, "tz::geo::Vec4 Addition yielded unexpected value. Expected ", 9, ", but got ", d[2]);
		topaz_expect(test_case, d[3] == 9.0f, "tz::geo::Vec4 Addition yielded unexpected value. Expected ", 9, ", but got ", d[3]);
	}

	// Subtraction
	{
		const tz::Vec4 d = b - a;
		constexpr tz::Vec4 e{{7.0f, 5.0f, 3.0f, 1.0f}};
		for(std::size_t i = 0; i < 4; i++)
			topaz_expect(test_case, d[i] == e[i], "tz::geo::Vec4 Subtraction yielded unexpected value. Expected ", e[i], ", but got ", d[i]);
	}
	return test_case;
}

tz::test::Case dot()
{
	tz::test::Case test_case("tz::geo::Vec4 Dot Product Tests");

	tz::Vec4 a{18.0f, 39.0f, 31.0f, 1.0f};
	tz::Vec4 b{101.0f, 58.0f, 75.0f, 1.0f};
	tz::Vec4 c{9.0f, 2.0f, 7.0f, 1.0f};
	tz::Vec4 d{4.0f, 8.0f, 10.0f, 1.0f};

	topaz_expect(test_case, a.length() == 52.981128f, "tz::gl::Vec4 Produced incorrect magnitude of: ", a.length());
	topaz_expect(test_case, a.dot(b) == 6406, "tz::gl::Vec4 Dot Product yielded incorrect result. Expected ", 6406.0f, ", but got ", a.dot(b));
	topaz_expect(test_case, c.dot(d) == 123.0f, "tz::gl::Vec4 Dot Product yielded incorrect result. Expected ", 123.0f, ", but got ", c.dot(d));

	return test_case;
}

tz::test::Case cross()
{
	tz::test::Case test_case("tz::geo::Vec3 Cross Product Tests");
	tz::Vec3 a(2.0f, 3.0f, 4.0f);
	tz::Vec3 b(5.0f, 6.0f, 7.0f);
	tz::Vec3 c = tz::cross(a, b);
	topaz_expect(test_case, c[0] == -3, "Cross Product incorrect.");
	topaz_expect(test_case, c[1] == 6, "Cross Product incorrect.");
	topaz_expect(test_case, c[2] == -3, "Cross Product incorrect.");
	// (2, 3, 4) × (5, 6, 7) = (−3, 6, −3)
	return test_case;
}

int main()
{
	tz::test::Unit vec;

	vec.add(addition_subtraction());
	vec.add(dot());
	vec.add(cross());

	return vec.result();
}