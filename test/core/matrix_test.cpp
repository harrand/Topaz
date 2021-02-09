//
// Created by Harrand on 06/01/2020.
//

#include "test_framework.hpp"
#include "core/tz.hpp"
#include "core/matrix.hpp"
#include <string>
#include <cstring>

TZ_TEST_BEGIN(identity)
	tz::Mat4 m = tz::Mat4::identity();
	for(std::size_t i = 0; i < 4; i++)
	{
		for(std::size_t j = 0; j < 4; j++)
		{
			if(i == j)
			{
				topaz_expect(m(i, j) == 1, "Malformed identity matrix. Expected m(", i, ", ", j, ") == 1, but it was ", m(i, j));
			}
			else
			{
				topaz_expect(m(i, j) == 0, "Malformed identity matrix. Expected m(", i, ", ", j, ") == 0, but it was ", m(i, j));
			}
		}
	}
TZ_TEST_END

TZ_TEST_BEGIN(addition)
	tz::Mat4 m;
	for(std::size_t i = 0; i < 4; i++)
	{
		for(std::size_t j = 0; j < 4; j++)
		{
			m(i, j) = (i*4) + j;
		}
	}

	m.debug_print();

	tz::Mat4 n;
	for(std::size_t i = 0; i < 4; i++)
	{
		for(std::size_t j = 0; j < 4; j++)
		{
			n(i, j) = -static_cast<float>((i*4) + j);
		}
	}

	tz::Mat4 o = m + n;

	for(std::size_t i = 0; i < 4; i++)
	{
		for(std::size_t j = 0; j < 4; j++)
		{
			topaz_expect(o(i, j) == 0, "Zero matrix unexpectedly had value ", o(i, j), " when it should be zero at: (", i, ", ", j, ")");
		}
	}
TZ_TEST_END

TZ_TEST_BEGIN(inversion)
	
	// Get the identity matrix.
	tz::Mat4 id = tz::Mat4::identity();
	// Inverting the identity matrix must yield the identity matrix.
	topaz_expect(id == id.inverse(), "Inverse of I != I... We have broken the rules of maths...");
	
	tz::Mat4 fours;
	for(std::size_t i = 0; i < 4; i++)
	{
		for(std::size_t j = 0; j < 4; j++)
		{
			fours(i, j) = -4;
		}
	}
	fours(0, 3) = 4;
	fours(1, 2) = 4;
	fours(2, 1) = 4;
	fours(3, 0) = 4;

	float sixteenth = 1.0f/16.0f;
	/*
	 * fours:
	 * |-4   -4   -4   4|
	 * |-4   -4   4   -4|
	 * |-4   4   -4   -4|
	 * |4   -4   -4   -4|
	 * 
	 * inverse fours:
	 * |-1/16   -1/16   -1/16   1/16|
	 * |-1/16   -1/16   1/16   -1/16|
	 * |-1/16   1/16   -1/16   -1/16|
	 * |1/16   -1/16   -1/16   -1/16|
	 */

	tz::Mat4 sixteenths = fours.inverse();
	for(std::size_t i = 0; i < 4; i++)
	{
		for(std::size_t j = 0; j < 4; j++)
		{
			if((i == 0 && j == 3) || (i == 1 && j == 2) || (i == 2 && j == 1) || (i == 3 && j == 0))
				topaz_expect(sixteenths(i, j) == sixteenth, "Inverse Matrix incorrect. Expected value ", sixteenth, " at (", i, ", ", j, "), but got value ", sixteenths(i, j));
			else
				topaz_expect(sixteenths(i, j) == -sixteenth, "Inverse Matrix incorrect. Expected value ", -sixteenth, " at (", i, ", ", j, "), but got value ", sixteenths(i, j));
		}
	}
TZ_TEST_END

TZ_TEST_BEGIN(column_major)
	tz::Mat4 order{{std::array<float, 4>{0, 1, 2, 3}, {4, 5, 6, 7}, {8, 9, 10, 11}, {12, 13, 14, 15}}};
	/*
	|0   4   8   12|
	|1   5   9   13|
	|2   6   10   14|
	|3   7   11   15|
	*/

	topaz_expect(order(0, 1) == 4.0f, "Expected ", 4.0f, " but got ", order(0, 1));
	topaz_expect(order(0, 2) == 8.0f, "Expected ", 8.0f, " but got ", order(0, 2));
	topaz_expect(order(3, 1) == 7.0f, "Expected ", 7.0f, " but got ", order(3, 1));
TZ_TEST_END

int main()
{
	tz::test::Unit mat;

	mat.add(identity());
	mat.add(addition());
	mat.add(inversion());
	mat.add(column_major());

	return mat.result();
}