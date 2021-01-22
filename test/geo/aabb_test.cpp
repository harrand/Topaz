//
// Created by Harrand on 22/01/2021.
//

#include "test_framework.hpp"
#include "geo/boundary/aabb.hpp"

tz::test::Case aabb_point_intersection()
{
	tz::test::Case test_case("tz::geo BounaryAABB Point Intersection Tests");

	tz::Vec3 min{-1.0f, -1.0f, -1.0f};
	tz::Vec3 max{1.0f, 1.0f, 1.0f};
	tz::geo::BoundaryAABB aabb{min, max};
	topaz_expect(test_case, aabb.collides(tz::Vec3{0.0f, 0.0f, 0.0f}), "AABB wrongly considered not colliding with the origin.");
	topaz_expect(test_case, !aabb.collides(tz::Vec3{10.0f, 10.0f, 10.0f}), "AABB wrongly considered colliding with a faraway point.");

	// But does it collide with itself? It should do.
	topaz_expect(test_case, aabb.collides(min) && aabb.collides(max), "AABB wrongly considered not to collide with its own min/max.");

	return test_case;
}

int main()
{
	tz::test::Unit aabb;

	aabb.add(aabb_point_intersection());

	return aabb.result();
}