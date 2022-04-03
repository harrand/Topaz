#include "phys/colliders/box.hpp"

void box_colliders();

int main()
{
	box_colliders();
}

void box_colliders()
{
	tz::phys::ColliderAABB box0
	{
		{.body = {}},
		{-0.5f, -0.5f, -0.5f},
		{0.5f, 0.5f, 0.5f}
	};
	tz_assert(box0.get_volume() == std::sqrt(3.0f), "Box Volume was incorrect. Expected %.2f, got %.2f", std::sqrt(3.0f), box0.get_volume());

	tz::phys::ColliderAABB box1
	{
		{.body = {.transform = {.position = {-0.5f, 0.0f, 0.0f}}}},
		{-0.5f, -0.5f, -0.5f},
		{0.5f, 0.5f, 0.5f}
	};
	tz_assert(box1.get_volume() == std::sqrt(3.0f), "Box Volume was incorrect. Expected %.2f, got %.2f", std::sqrt(3.0f), box1.get_volume());

	tz::phys::ColliderAABB box2
	{
		{.body = {.transform = {.position = {-100.0f, 0.0f, 0.0f}}}},
		{-0.5f, -0.5f, -0.5f},
		{0.5f, 0.5f, 0.5f}
	};

	tz::phys::ColliderPoint p0;
	tz::phys::ColliderPoint p1
	{
		{.body = {}},
		{0.0f, 100.0f, 0.0f}
	};

	// AABB-Point (Success)
	tz::phys::Collision collision0{.a = box0, .b = p0};
	tz::phys::CollisionInfo result0 = tz::phys::collide(collision0);
	tz_assert(result0.collides, "Point {%.2f, %.2f, %.2f} wrongly considered not to be within box.", p0.get_position()[0], p0.get_position()[1], p0.get_position()[2]);

	// AABB-Point (Fail)
	tz::phys::Collision collision1{.a = box0, .b = p1};
	tz::phys::CollisionInfo result1 = tz::phys::collide(collision1);
	tz_assert(!result1.collides, "Point {%.2f, %.2f, %.2f} wrongly considered to be within box.", p1.get_position()[0], p1.get_position()[1], p1.get_position()[2]);

	// AABB-AABB (Success)
	tz::phys::Collision collision2{.a = box0, .b = box1};
	tz::phys::CollisionInfo result2 = tz::phys::collide(collision2);
	tz_assert(result2.collides, "Boxes wrongly considered not to be colliding.");

	// AABB-AABB (Fail)
	tz::phys::Collision collision3{.a = box0, .b = box2};
	tz::phys::CollisionInfo result3 = tz::phys::collide(collision3);
	tz_assert(!result3.collides, "Boxes wrongly considered to be colliding.");
}
