#ifndef TOPAZ_CORE_QUATERNION_HPP
#define TOPAZ_CORE_QUATERNION_HPP
#include "tz/core/vector.hpp"
#include "tz/core/matrix.hpp"

namespace tz
{
	/**
	 * @ingroup tz_core_math
	 * @brief Quaternion. Represents a rotation in 3D space.
	 */
	struct quat : public tz::v4f
	{
		/// Retrieve the identity quaternion, that is - represents no transformation.
		static quat iden()
		{
			return {tz::v4f{0.0f, 0.0f, 0.0f, 1.0f}};
		}
		quat() = default;
		/// Create a quaternion directly from xyzw components.
		quat(tz::v4f vec);
		/// Create a quaternion that represents a rotation about a certain axis.
		static quat from_axis_angle(tz::v3f axis, float angle);
		/// Create a quaternion that represents a rotation equivalent to the provided euler angles.
		static quat from_euler_angles(tz::v3f euler_angles);
		/// Create a rotation matrix that transforms identically to the quaternion.
		tz::m4f matrix() const;
		/// Create a quaternion that causes the inverse transformation. Such that the product of this and the result are the identity quaternion.
		quat inverse() const;
		/// Combine one quaternion with another, producing a result equal to applying both rotations.
		quat combine(const quat& rhs) const;
		/// Rotate a 3D position by this quaternion.
		tz::v3f rotate(tz::v3f pos) const;
		/// Retrieve a normalised copy of the quaternion.
		quat normalise() const;
		/// Retrieve a quaternion that represents a spherical interpolation between this quaternion and another, based upon a given factor.
		quat slerp(const quat& rhs, float factor) const;

		quat& operator*=(const quat& rhs);
		quat operator*(const quat& rhs) const{auto cpy = *this; return cpy *= rhs;}
	};
}

#endif // TOPAZ_CORE_QUATERNION_HPP