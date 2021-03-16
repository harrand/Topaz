#ifndef TOPAZ_PHYS_SOLVER_GENERIC_HPP
#define TOPAZ_PHYS_SOLVER_GENERIC_HPP
#include "phys/collision.hpp"

namespace tz::phys
{
    class IResolver
    {
    public:
        virtual void solve_all(CollisionList& collisions, float delta_millis) = 0;
    };
}

#endif // TOPAZ_PHYS_SOLVER_GENERIC_HPP