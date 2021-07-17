//
// Created by Harrand on 06/01/2020.
//

#include "core/tz.hpp"
#include "core/matrix.hpp"
#include <string>
#include <cstring>

void identity()
{
	tz::Mat4 m = tz::Mat4::identity();
	for(std::size_t i = 0; i < 4; i++)
	{
		for(std::size_t j = 0; j < 4; j++)
		{
			if(i == j)
			{
				tz_assert(m(i, j) == 1, "Malformed identity matrix. Expected m(%zu, %zu) == 1, but it was %g", i, j, m(i, j));
			}
			else
			{
				tz_assert(m(i, j) == 0, "Malformed identity matrix. Expected m(%zu, %zu) == 0, but it was %g", i, j, m(i, j));
			}
		}
	}
}

void addition()
{
	tz::Mat4 m;
	for(std::size_t i = 0; i < 4; i++)
	{
		for(std::size_t j = 0; j < 4; j++)
		{
			m(i, j) = static_cast<float>((i*4) + j);
		}
	}

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
			tz_assert(o(i, j) == 0, "Zero matrix unexpectedly had value %g when it should be zero at: (%zu, %zu)", o(i, j), i, j);
		}
	}
}

void inversion()
{	
	// Get the identity matrix.
	tz::Mat4 id = tz::Mat4::identity();
	// Inverting the identity matrix must yield the identity matrix.
	tz_assert(id == id.inverse(), "Inverse of I != I... We have broken the rules of maths...");
	
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
			{
				tz_assert(sixteenths(i, j) == sixteenth, "Inverse Matrix incorrect. Expected value %g at (%zu, %zu), but got value ", sixteenth, i, j, sixteenths(i, j));
			}
			else
			{
				tz_assert(sixteenths(i, j) == -sixteenth, "Inverse Matrix incorrect. Expected value %g at (%g, %g), but got value ", -sixteenth, i, j, sixteenths(i, j));
			}
		}
	}
}

void column_major()
{
	tz::Mat4 order{{std::array<float, 4>{0, 1, 2, 3}, {4, 5, 6, 7}, {8, 9, 10, 11}, {12, 13, 14, 15}}};
	/*
	|0   4   8   12|
	|1   5   9   13|
	|2   6   10   14|
	|3   7   11   15|
	*/

	tz_assert(order(0, 1) == 4.0f, "Expected %g but got %g", 4.0f, order(0, 1));
	tz_assert(order(0, 2) == 8.0f, "Expected %g but got %g", 8.0f, order(0, 2));
	tz_assert(order(3, 1) == 7.0f, "Expected %g but got %g", 7.0f, order(3, 1));
}

int main()
{
	identity();
	addition();
	inversion();
	column_major();
}