#ifndef TOPAZ_GEO_QUATERNION_HPP
#define TOPAZ_GEO_QUATERNION_HPP
#include "geo/vector.hpp"

namespace tz
{
	class Quaternion : private tz::Vec4
	{
	public:
		Quaternion();
		/**
		 * Construct a quaternion from a rotation axis and angle.
		 * @param axis Rotation axis. This will be normalised so no need to do that yourself.
		 * @param angle Angle, in radians.
		 */
		Quaternion(tz::Vec3 axis, float angle);
		/**
		 * Calculate the conjugate of a quaternion.
		 * @return A copy of *this, but with the polarity reversed.
		 */
		Quaternion conjugated() const;
		/**
		 * Calculate the inverse of a quaternion.
		 * @return A copy of *this, but inversed.
		 */
		Quaternion inversed() const;
		using tz::Vec4::normalise;
		using tz::Vec4::normalised;

		Quaternion& operator*=(const Quaternion& rhs);
		Quaternion operator*(const Quaternion& rhs) const;
	private:
		static void swap(Quaternion& lhs, Quaternion& rhs);
	};
}

#endif //TOPAZ_GEO_QUATERNION_HPP