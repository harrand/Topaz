#ifndef TOPAZ_CORE_QUATERNION_HPP
#define TOPAZ_CORE_QUATERNION_HPP
#include "tz/core/vector.hpp"
#include "tz/core/matrix.hpp"

namespace tz
{
	struct quat : public tz::v4f
	{
		static quat iden()
		{
			return {tz::v4f{0.0f, 0.0f, 0.0f, 1.0f}};
		}
		quat() = default;
		quat(tz::v4f vec);
		static quat from_axis_angle(tz::v3f axis, float angle);
		static quat from_euler_angles(tz::v3f euler_angles);
		tz::m4f matrix() const;
		quat inverse() const;
		quat combine(const quat& rhs) const;
		tz::v3f rotate(tz::v3f pos) const;
		quat normalise() const;
		quat slerp(const quat& rhs, float factor) const;

		quat& operator*=(const quat& rhs);
		quat operator*(const quat& rhs) const{auto cpy = *this; return cpy *= rhs;}
	};
}

#endif // TOPAZ_CORE_QUATERNION_HPP