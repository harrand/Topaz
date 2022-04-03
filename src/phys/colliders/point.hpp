#ifndef TOPAZ_PHYS_COLLIDERS_POINT_HPP
#define TOPAZ_PHYS_COLLIDERS_POINT_HPP
#include "phys/colliders/common.hpp"

namespace tz::phys
{
	/**
	 * @ingroup tz_phys_collider
	 * Represents an infinitessimally small point in 3D space.
	 */
	struct ColliderPoint : public ColliderBase
	{
		/// Offset from the transform position. Default {0, 0, 0}
		tz::Vec3 offset = {0.0f, 0.0f, 0.0f};

		/**
		 * Retrieve the position in world space (transform position + offset).
		 * @return Position, in world space.
		 */
		tz::Vec3 get_position() const;
	};
}

#endif // TOPAZ_PHYS_COLLIDERS_POINT_HPP
