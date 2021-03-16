#ifndef TOPAZ_PHYS_IMPULE_RESOLVER_HPP
#define TOPAZ_PHYS_IMPULE_RESOLVER_HPP
#include "phys/resolvers/generic.hpp"

namespace tz::phys
{
    class ImpulseResolver : public IResolver
    {
        virtual void solve_all(CollisionList& collisions, float delta_millis) override;
    };
}

#endif // TOPAZ_PHYS_IMPULE_RESOLVER_HPP