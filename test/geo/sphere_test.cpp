//
// Created by Harrand on 24/01/2021.
//

#include "test_framework.hpp"
#include "geo/boundary/sphere.hpp"

TZ_TEST_BEGIN(sphere_point_intersection)
	tz::geo::BoundarySphere origin3{{0.0f, 0.0f, 0.0f}, 3.0f};
	topaz_expect(origin3.collides(tz::Vec3{0.0f, 0.0f, 0.0f}), "AABB wrongly considered not colliding with the origin.");
	topaz_expect(!origin3.collides(tz::Vec3{10.0f, 10.0f, 10.0f}), "AABB wrongly considered colliding with a faraway point.");
TZ_TEST_END

TZ_TEST_BEGIN(sphere_sphere_intersection)
	/*
		       *-*      |						 5
		     *     *    |						 4
		  *a         *  |						 3
		*       b      *|*						 2
		  *          *  |  *						 1
	---------*------*---c---*----------------	 0
		 	   *-*	 *	|  *	 *-*    		-1
					   *|*	   *-----*  		-2
						|	  *---d---* 		-3
						|	   *-----*         	-4
						|        *-*	    	-5
	-5 -4  -3  -2  -1   0   1   2   3   4   5
	*/
    tz::geo::BoundarySphere a{{-3.0f, 3.0f, 0.0f}, 0.5f};
    tz::geo::BoundarySphere b{{-2.0f, 2.0f, 0.0f}, 2.0f};
    tz::geo::BoundarySphere c{{0.0f, 0.0f, 0.0f}, 1.0f};
    tz::geo::BoundarySphere d{{2.5f, -3.0f, 0.0f}, 1.0f};

	// Sphere A
	topaz_expect(a.collides(b), "Box A wrongly considered not colliding with Box B");
	topaz_expect(b.collides(a), "Box B wrongly considered not colliding with Box A");
	
	topaz_expect(!a.collides(c), "Box A wrongly considered colliding with Box C.");
	topaz_expect(!c.collides(a), "Box C wrongly considered colliding with Box A.");

	topaz_expect(!a.collides(d), "Box A wrongly considered colliding with Box D.");
	topaz_expect(!d.collides(a), "Box D wrongly considered colliding with Box A.");

	// Sphere B
	topaz_expect(b.collides(c), "Box B wrongly considered not colliding with Box C.");
	topaz_expect(c.collides(b), "Box C wrongly considered not colliding with Box B.");

	topaz_expect(!b.collides(d), "Box B wrongly considered colliding with Box D.");
	topaz_expect(!d.collides(b), "Box D wrongly considered colliding with Box B.");

	// Sphere C
	topaz_expect(!c.collides(d), "Box C wrongly considered colliding with Box D.");
	topaz_expect(!d.collides(c), "Box D wrongly considered colliding with Box C.");
TZ_TEST_END

int main()
{
	tz::test::Unit sphere;

	sphere.add(sphere_point_intersection());
	sphere.add(sphere_sphere_intersection());

	return sphere.result();
}