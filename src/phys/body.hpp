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
		/**
		 * Location of the body in the world. Defaults to origin, with no rotation and unit scale.
		 */
		Transform transform = {};
		/**
		 * Velocity. Defaults to {0, 0, 0}.
		 */
		tz::Vec3 velocity{0.0f, 0.0f, 0.0f};
		/**
		 * Acceleration. Defaults to {0, 0, 0}.
		 */
		tz::Vec3 acceleration{0.0f, 0.0f, 0.0f};

		/**
		 * Update body by performing motion integration.
		 */
		void update(float delta_millis);
		/**
		 * Retrieve the current speed of the body.
		 */
		float get_speed() const;
		/**
		 * Retrieve the length of the current acceleration of the body.
		 */
		float get_acceleration_magnitude() const;
	};
}

#endif // TOPAZ_PHYS_BODY_HPP
