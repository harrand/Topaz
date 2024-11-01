#ifndef TOPAZ_CORE_TRS_HPP
#define TOPAZ_CORE_TRS_HPP
#include "tz/core/vector.hpp"
#include "tz/core/quaternion.hpp"
#include "tz/core/matrix.hpp"

namespace tz
{
	tz::m4f matrix_translate(tz::v3f translate);
	tz::m4f matrix_scale(tz::v3f scale);
	tz::m4f matrix_ortho(float left, float right, float top, float bottom, float near, float far);

	struct trs
	{
		tz::v3f translate = tz::v3f::zero();
		tz::quat rotate = tz::quat::iden();
		tz::v3f scale = tz::v3f::filled(1.0f);

		trs lerp(const trs& rhs, float factor) const;
		m4f matrix() const;
		static trs from_matrix(m4f mat);

		trs inverse() const;
		trs combine(const trs& rhs);

		bool operator==(const trs& rhs) const = default;
	};
}

#endif // TOPAZ_CORE_TRS_HPP