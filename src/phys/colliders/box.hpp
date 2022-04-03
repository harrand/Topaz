#ifndef TOPAZ_PHYS_COLLIDERS_BOX_HPP
#define TOPAZ_PHYS_COLLIDERS_BOX_HPP
#include "phys/colliders/common.hpp"
#include "phys/colliders/point.hpp"

namespace tz::phys
{
	struct ColliderAABB : public ColliderBase
	{
		tz::Vec3 offset_min;
		tz::Vec3 offset_max;

		float get_volume() const;
		tz::Vec3 get_min() const;
		tz::Vec3 get_max() const;
		tz::Vec3 get_offset_midpoint() const;
		tz::Vec3 get_midpoint() const;
	};

	static_assert(Collider<ColliderAABB>);

	// AABB-Point Collision
	template<>
	struct CollisionInfo<ColliderAABB, ColliderPoint>
	{
		bool collides;
		tz::Vec3 offset_beyond_min;
	};

	template<>
	CollisionInfo<ColliderAABB, ColliderPoint> collide(Collision<ColliderAABB, ColliderPoint> collision);

	// AABB-AABB Collision

	template<>
	struct CollisionInfo<ColliderAABB, ColliderAABB>
	{
		bool collides;
		tz::Vec3 penetration_min;
		tz::Vec3 penetration_max;
	};

	template<>
	CollisionInfo<ColliderAABB, ColliderAABB> collide(Collision<ColliderAABB, ColliderAABB> collision);
}

#endif // TOPAZ_PHYS_COLLIDERS_BOX_HPP
