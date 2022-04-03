#ifndef TOPAZ_PHYS_COLLIDERS_POINT_HPP
#define TOPAZ_PHYS_COLLIDERS_POINT_HPP
#include "phys/colliders/common.hpp"

namespace tz::phys
{
	struct ColliderPoint : public ColliderBase
	{
		tz::Vec3 offset = {0.0f, 0.0f, 0.0f};

		tz::Vec3 get_position() const;
	};
}

#endif // TOPAZ_PHYS_COLLIDERS_POINT_HPP
