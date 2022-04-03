#ifndef TOPAZ_PHYS_COLLIDERS_BOX_HPP
#define TOPAZ_PHYS_COLLIDERS_BOX_HPP
#include "phys/colliders/common.hpp"
#include "phys/colliders/point.hpp"

namespace tz::phys
{
	/**
	 * @ingroup tz_phys_collider
	 * Represents an axis-aligned bounding box in 3D space.
	 */
	struct ColliderAABB : public ColliderBase
	{
		/// Offset from the transform position representing the min extent.
		tz::Vec3 offset_min;
		/// Offset from the transform position representing the max extent.
		tz::Vec3 offset_max;

		/**
		 * Retrieve the volume of the box.
		 */
		float get_volume() const;
		/**
		 * Retrieve the position of the minimum extent, in world space (transform position + offset_min).
		 */
		tz::Vec3 get_min() const;
		/**
		 * Retrieve the position of the maximum extent, in world space (transform position + offset_max).
		 */
		tz::Vec3 get_max() const;
		/**
		 * Retrieve the midpoint offset, meaning the midpoint between the offset minimum and the offset maximum. If the box is a cube or square, this will be {0, 0, 0}, unaffected by the transform position.
		 */
		tz::Vec3 get_offset_midpoint() const;
		/**
		 * Retrieve the midpoint of the box, in world-space.
		 */
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
