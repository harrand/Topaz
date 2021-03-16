#ifndef TOPAZ_PHYS_POSITION_RESOLVER_HPP
#define TOPAZ_PHYS_POSITION_RESOLVER_HPP
#include "phys/resolvers/generic.hpp"

namespace tz::phys
{
    class PositionResolver : public IResolver
    {
        virtual void solve_all(CollisionList& collisions, float delta_millis) override;
    };
}

#endif // TOPAZ_PHYS_POSITION_RESOLVER_HPP