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
		#if TZ_DEBUG
		tz::vec2 dbgui_slider_scale = tz::vec2::filled(1.0f);
		#endif // TZ_DEBUG

		trs lerp(const trs& rhs, float factor) const;
		tz::mat4 matrix() const;
		trs& combine(const trs& rhs);
		trs combined(const trs& rhs) const;
		void dbgui();
	};
}

#endif // TOPAZ_CORE_DATA_TRS_HPP