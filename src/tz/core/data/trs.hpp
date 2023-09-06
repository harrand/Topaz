#ifndef TOPAZ_CORE_DATA_TRS_HPP
#define TOPAZ_CORE_DATA_TRS_HPP
#include "tz/core/data/quat.hpp"

namespace tz
{
	struct trs
	{
		tz::vec3 translate = tz::vec3::zero();
		tz::quat rotate = {0.0f, 0.0f, 0.0f, 1.0f};
		tz::vec3 scale = tz::vec3::filled(1.0f);
		#if TZ_DEBUG
		tz::vec2 dbgui_slider_scale = tz::vec2::filled(1.0f);
		#endif // TZ_DEBUG

		trs lerp(const trs& rhs, float factor) const;
		tz::mat4 matrix() const;
		static trs from_matrix(tz::mat4 mat);
		trs& combine(const trs& rhs);
		trs combined(const trs& rhs) const;
		void dbgui();
		bool operator==(const trs& rhs) const = default;
	};
}

namespace std
{
	template<>
	struct hash<tz::trs>
	{
		std::size_t operator()(const tz::trs& t) const
		{
			std::size_t ret = 0;
			ret ^= std::hash<tz::vec3>{}(t.translate);
			ret ^= std::hash<tz::quat>{}(t.rotate);
			ret ^= std::hash<tz::vec3>{}(t.scale);
			return ret;
		}
	};
}

#endif // TOPAZ_CORE_DATA_TRS_HPP