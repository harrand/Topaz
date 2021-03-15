#ifndef TOPAZ_PHYS_SPHERE_COLLIDER_HPP
#define TOPAZ_PHYS_SPHERE_COLLIDER_HPP
#include "phys/colliders/generic.hpp"
#include "geo/boundary/sphere.hpp"

namespace tz::phys
{
    class SphereCollider : public ICollider, public tz::geo::BoundarySphere
    {
    public:
        SphereCollider(tz::Vec3 centre, float radius);

        virtual ColliderType get_type() const override{ return ColliderType::Sphere; }
        virtual CollisionPoint test_against(const ICollider& rhs) const;
    private:

    };
}

#endif // TOPAZ_PHYS_SPHERE_COLLIDER_HPP