#ifndef TOPAZ_PHYS_COLLISION_HPP
#define TOPAZ_PHYS_COLLISION_HPP
#include "core/vector.hpp"
#include <vector>

namespace tz::phys
{
    // Pre definition
    struct Body;

    class CollisionPoint
    {
    public:
        CollisionPoint(tz::Vec3 a, tz::Vec3 b, bool has_collision);
        tz::Vec3 normal() const;
        float depth() const;
        bool collides() const;
    private:
        tz::Vec3 a; // furthest point of A into B
        tz::Vec3 b; // furthest point of B into A
        bool has_collision;
    };

    struct CollisionSituation
    {
        Body& a;
        Body& b;
        CollisionPoint point;
    };

    using CollisionList = std::vector<CollisionSituation>;
}

#endif // TOPAZ_PHYS_COLLISION_HPP