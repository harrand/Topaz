#ifndef TOPAZ_PHYS_TRANSFORM_HPP
#define TOPAZ_PHYS_TRANSFORM_HPP
#include "core/vector.hpp"

namespace tz::phys
{
	/**
	 * @ingroup tz_phys
	 * Represents the position, rotation and scale of some body in 3D space.
	 */
	struct Transform
	{
		/// Position, in world-space.
		tz::Vec3 position{0.0f, 0.0f, 0.0f};
		/// Rotation, in world-space (radians).
		tz::Vec3 rotation{0.0f, 0.0f, 0.0f};
		/// Scale.
		tz::Vec3 scale{1.0f, 1.0f, 1.0f};
	};
}

#endif // TOPAZ_PHYS_TRANSFORM_HPP
