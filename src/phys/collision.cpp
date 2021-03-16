#include "phys/collision.hpp"

namespace tz::phys
{
    CollisionPoint::CollisionPoint(tz::Vec3 a, tz::Vec3 b, bool has_collision): a(a), b(b), has_collision(has_collision){}
 
    tz::Vec3 CollisionPoint::normal() const
    {
        return this->b - this->a;
    }

    float CollisionPoint::depth() const
    {
        return this->normal().length();
    }

    bool CollisionPoint::collides() const
    {
        return this->has_collision;
    }
}