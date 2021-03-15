#include "phys/colliders/sphere.hpp"
#include "algo/physics_collisions.hpp"

namespace tz::phys
{
    SphereCollider::SphereCollider(tz::Vec3 centre, float radius): ICollider(), tz::geo::BoundarySphere(centre, radius){}

    CollisionPoint SphereCollider::test_against(const ICollider& rhs) const
    {
        auto get_worldspace_collider = [](SphereCollider collider)->SphereCollider
        {
            collider.set_centre(collider.get_centre() + collider.get_parent_position());
            return collider;
        };


        SphereCollider lhs_worldspace = get_worldspace_collider(*this);
        SphereCollider rhs_worldspace = get_worldspace_collider(static_cast<const SphereCollider&>(rhs));
        switch(rhs.get_type())
        {
            case ColliderType::Sphere:
                return tz::algo::collision::sphere_sphere(lhs_worldspace, rhs_worldspace);
            break;
            default:
                topaz_assert(false, "Sphere-X collision -- Not yet implemented.");
                return {tz::Vec3{}, tz::Vec3{}, false};
            break;
        }
    }
}