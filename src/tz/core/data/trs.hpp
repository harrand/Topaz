#ifndef TOPAZ_CORE_DATA_TRS_HPP
#define TOPAZ_CORE_DATA_TRS_HPP
#include "tz/core/data/quat.hpp"

namespace tz
{
	struct trs
	{
		tz::vec3 translate = tz::vec3::zero();
		tz::quat rotate = tz::quat::zero();
		tz::vec3 scale = tz::vec3::filled(1.0f);

		trs lerp(const trs& rhs, float factor) const;
		tz::mat4 matrix() const;
		trs& combine(const trs& rhs);
		trs combined(const trs& rhs) const;
	};
}

#endif // TOPAZ_CORE_DATA_TRS_HPP