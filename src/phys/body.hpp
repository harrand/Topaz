#ifndef TOPAZ_PHYS_BODY_HPP
#define TOPAZ_PHYS_BODY_HPP
#include "phys/transform.hpp"

namespace tz::phys
{
	/**
	 * @ingroup tz_phys
	 * Represents a body in 3D space that can move.
	 */
	struct Body
	{
		Transform transform = {};
		tz::Vec3 velocity{0.0f, 0.0f, 0.0f};
		tz::Vec3 acceleration{0.0f, 0.0f, 0.0f};

		void update(float delta_millis);
		float get_speed() const;
		float get_acceleration_magnitude() const;
	};
}

#endif // TOPAZ_PHYS_BODY_HPP
