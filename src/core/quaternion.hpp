#ifndef TOPAZ_CORE_QUATERNION_HPP
#define TOPAZ_CORE_QUATERNION_HPP
#include "core/vector.hpp"
#include "core/matrix.hpp"

namespace tz
{

	/**
	 * \addtogroup tz_core Topaz Core Library (tz::core)
	 * A collection of platform-agnostic core interfaces.
	 * @{
	 */

	struct AxisAndAngle
	{
		tz::Vec3 axis;
		float angle;	
	};

	class Quaternion : private tz::Vec4
	{
	public:
		/**
		 * Construct a quaternion which performs no rotation.
		 */
		Quaternion();
		/**
		 * Construct a quaternion from a rotation axis and angle.
		 * @param axis Rotation axis. This will be normalised so no need to do that yourself.
		 * @param angle Angle, in radians.
		 */
		Quaternion(tz::Vec3 axis, float angle);
		/**
		 * Construct a quaternion from a set of euler angles.
		 * @param euler_angles Rotations in each dimension, in radians.
		 */
		Quaternion(tz::Vec3 euler_angles);
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
		void normalise();
		Quaternion normalised() const;
		/**
		 * Convert a quaternion to a rotational matrix.
		 * @return Rotation matrix representing identical transformation to this quaternion.
		 */
		explicit operator tz::Mat4() const;

		Quaternion& operator*=(const Quaternion& rhs);
		Quaternion operator*(const Quaternion& rhs) const;
		bool operator==(const Quaternion& rhs) const;

		// More explicit conversions.
		static Quaternion from_axis(AxisAndAngle rotation);
		AxisAndAngle to_axis() const;

		static Quaternion from_eulers(tz::Vec3 euler_angles);
		tz::Vec3 to_eulers() const;

		static Quaternion from_matrix(tz::Mat4 rotation);
		tz::Mat4 to_matrix() const;
	private:
		Quaternion(float x, float y, float z, float w);
		static void swap(Quaternion& lhs, Quaternion& rhs);
		using tz::Vec4::normalise;
		using tz::Vec4::normalised;
	};

	/**
	 * @}
	 */
}

#endif //TOPAZ_CORE_QUATERNION_HPP