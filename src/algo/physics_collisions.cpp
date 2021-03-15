#include "algo/physics_collisions.hpp"
#include "core/debug/print.hpp"

namespace tz::algo::collision
{
    tz::phys::CollisionPoint sphere_sphere(const Sphere& a, const Sphere& b)
    {
        tz::Vec3 a_centre_to_b_centre = b.get_centre() - a.get_centre();
        tz::Vec3 furthest_a_into_b = a.get_centre() + (a_centre_to_b_centre.normalised() * a.get_radius());
        tz::Vec3 furthest_b_into_a = b.get_centre() + ((a_centre_to_b_centre * -1.0f).normalised() * b.get_radius());
  
        bool collides = a_centre_to_b_centre.length() <= (a.get_radius() + b.get_radius());
        return {furthest_a_into_b, furthest_b_into_a, collides};
    }

    tz::phys::CollisionPoint aabb_aabb(const AABB& a, const AABB& b)
    {
        tz::Vec3 amin_bmax_diff = a.get_minimum() - b.get_maximum();
        tz::Vec3 amax_bmin_diff = a.get_maximum() - b.get_minimum();

        bool collides = true;
        for(std::size_t i = 0; i < 3; i++)
        {
            if(amin_bmax_diff[i] > 0.0f || amax_bmin_diff[i] < 0.0f)
            {
                collides = false;
            }
        }
        tz::Vec3 unimplemented{};
        tz::debug_printf("Warning: tz::algo::collision::aabb_aabb(...): This is unimplemented. Whether it collides is accurate but the normal/depth is not.");
        return {unimplemented, unimplemented, collides};
    }
}