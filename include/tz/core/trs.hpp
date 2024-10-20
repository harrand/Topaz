#ifndef TOPAZ_CORE_TRS_HPP
#define TOPAZ_CORE_TRS_HPP
#include "tz/core/vector.hpp"
#include "tz/core/matrix.hpp"

namespace tz
{
	struct trs
	{
		tz::v3f translate = tz::v3f::zero();
		// tz::quat rotate = tz::quat::iden();
		tz::v3f scale = tz::v3f::filled(1.0f);

		trs lerp(const trs& rhs, float factor) const;
		m4u matrix() const;
		static trs from_matrix(m4f mat);

		trs inverse() const;
		trs combine(const trs& rhs);

		bool operator==(const trs& rhs) const = default;
	};
}

#endif // TOPAZ_CORE_TRS_HPP