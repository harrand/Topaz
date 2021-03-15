#ifndef TOPAZ_ALGO_PHYSICS_COLLISIONS_HPP
#define TOPAZ_ALGO_PHYSICS_COLLISIONS_HPP
#include "geo/boundary/sphere.hpp"
#include "geo/boundary/aabb.hpp"
#include "phys/colliders/generic.hpp"

namespace tz::algo::collision
{
    using Sphere = tz::geo::BoundarySphere;
    using AABB = tz::geo::BoundaryAABB;

    tz::phys::CollisionPoint sphere_sphere(const Sphere& a, const Sphere& b);
    tz::phys::CollisionPoint aabb_aabb(const AABB& a, const AABB& b);
}

#endif // TOPAZ_ALGO_PHYSICS_COLLISIONS_HPP