#include "phys/colliders/box.hpp"

namespace tz::phys
{
	float ColliderAABB::get_volume() const
	{
		return (this->offset_max - this->offset_min).length();
	}

	tz::Vec3 ColliderAABB::get_min() const
	{
		return this->body.transform.position + this->offset_min;
	}

	tz::Vec3 ColliderAABB::get_max() const
	{
		return this->body.transform.position + this->offset_max;
	}

	tz::Vec3 ColliderAABB::get_offset_midpoint() const
	{
		return (this->offset_min + this->offset_max) * 0.5f;
	}

	tz::Vec3 ColliderAABB::get_midpoint() const
	{
		return (this->get_min() + this->get_max()) * 0.5f;
	}

	// AABB-Point Collision
	

	template<>
	CollisionInfo<ColliderAABB, ColliderPoint> collide(Collision<ColliderAABB, ColliderPoint> collision)
	{
		tz::Vec3 p = collision.b.get_position();
		float px = p[0], py = p[1], pz = p[2];
		tz::Vec3 min = collision.a.get_min();
		tz::Vec3 max = collision.a.get_max();
		return
		{
			.collides = (px >= min[0] && py >= min[1] && pz >= min[2]) && (px <= max[0] && py <= max[1] && pz <= max[2]),
			.offset_beyond_min = p - min
		};
	}

	// AABB-AABB Collision

	template<>
	CollisionInfo<ColliderAABB, ColliderAABB> collide(Collision<ColliderAABB, ColliderAABB> collision)
	{
		tz::Vec3 penetration_min, penetration_max;
		bool collidex = false, collidey = false, collidez = false;

		auto collide_axis = [&collision, &penetration_min, &penetration_max](std::size_t i, bool& collide)
		{

			bool rightwards = collision.a.get_min()[i] < collision.b.get_min()[i];
			bool right_collision = rightwards && collision.a.get_max()[i] >= collision.b.get_min()[0];
			if(right_collision)
			{
				// We're colliding on this axis.
				penetration_min[i] = collision.b.get_min()[i];
				penetration_max[i] = collision.a.get_max()[i];
				collide = true;
			}

			bool leftwards = !rightwards;
			bool left_collision = leftwards && collision.b.get_max()[i] >= collision.a.get_min()[i];
			if(left_collision)
			{
				// We're colliding on this axis.
				penetration_min[i] = collision.a.get_min()[i];
				penetration_max[i] = collision.b.get_max()[i];
				collide = true;
			}
		};

		collide_axis(0, collidex);
		collide_axis(1, collidey);
		collide_axis(2, collidez);

		return
		{
			.collides = collidex && collidey && collidez,
			.penetration_min = penetration_min,
			.penetration_max = penetration_max
		};
	}
}
