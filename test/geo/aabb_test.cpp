//
// Created by Harrand on 22/01/2021.
//

#include "test_framework.hpp"
#include "geo/boundary/aabb.hpp"

TZ_TEST_BEGIN(aabb_point_intersection)
	tz::Vec3 min{-1.0f, -1.0f, -1.0f};
	tz::Vec3 max{1.0f, 1.0f, 1.0f};
	tz::geo::BoundaryAABB aabb{min, max};
	topaz_expect(aabb.collides(tz::Vec3{0.0f, 0.0f, 0.0f}), "AABB wrongly considered not colliding with the origin.");
	topaz_expect(!aabb.collides(tz::Vec3{10.0f, 10.0f, 10.0f}), "AABB wrongly considered colliding with a faraway point.");

	// But does it collide with itself? It should do.
	topaz_expect(aabb.collides(min) && aabb.collides(max), "AABB wrongly considered not to collide with its own min/max.");
TZ_TEST_END

TZ_TEST_BEGIN(aabb_aabb_intersection)
	/*
		a *----*   c*--*|						 5
		  |    |	|  ||						 4
		  |   *+----*--*|						 3
		  *---|*	| b	|						 2
			  *-----*	|						 1
	--------------------+--------------------	 0
		 				|						-1
						|	*-------*			-2
						|	|	d	|			-3
						|	|		|			-4
						|	*-------*			-5
	-5 -4  -3  -2  -1   0   1   2   3   4   5
	*/
	tz::Vec3 a1{-3.5f, 2.0f, 0.0f};
	tz::Vec3 a2{-2.0f, 5.0f, 0.0f};
	tz::geo::BoundaryAABB a{a1, a2};

	tz::Vec3 b1{-2.5f, 1.5f, 0.0f};
	tz::Vec3 b2{-1.0f, 3.0f, 0.0f};
	tz::geo::BoundaryAABB b{b1, b2};

	tz::Vec3 c1{-1.0f, 3.0f, 0.0f};
	tz::Vec3 c2{-0.5f, 5.0f, 0.0f};
	tz::geo::BoundaryAABB c{c1, c2};

	tz::Vec3 d1{1.0f, -5.0f, 0.0f};
	tz::Vec3 d2{3.0f, -2.0f, 0.0f};
	tz::geo::BoundaryAABB d{d1, d2};

	// Box A
	topaz_expect(a.collides(b), "Box A wrongly considered not colliding with Box B");
	topaz_expect(b.collides(a), "Box B wrongly considered not colliding with Box A");
	
	topaz_expect(!a.collides(c), "Box A wrongly considered colliding with Box C.");
	topaz_expect(!c.collides(a), "Box C wrongly considered colliding with Box A.");

	topaz_expect(!a.collides(d), "Box A wrongly considered colliding with Box D.");
	topaz_expect(!d.collides(a), "Box D wrongly considered colliding with Box A.");

	// Box B
	topaz_expect(b.collides(c), "Box B wrongly considered not colliding with Box C.");
	topaz_expect(c.collides(b), "Box C wrongly considered not colliding with Box B.");

	topaz_expect(!b.collides(d), "Box B wrongly considered colliding with Box D.");
	topaz_expect(!d.collides(b), "Box D wrongly considered colliding with Box B.");

	// Box C
	topaz_expect(!c.collides(d), "Box C wrongly considered colliding with Box D.");
	topaz_expect(!d.collides(c), "Box D wrongly considered colliding with Box C.");
TZ_TEST_END

int main()
{
	tz::test::Unit aabb;

	aabb.add(aabb_point_intersection());
	aabb.add(aabb_aabb_intersection());

	return aabb.result();
}