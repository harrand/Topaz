#include "phys/colliders/point.hpp"

namespace tz::phys
{
	tz::Vec3 ColliderPoint::get_position() const
	{
		return this->body.transform.position + this->offset;
	}
}
