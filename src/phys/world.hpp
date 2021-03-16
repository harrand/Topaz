#ifndef TOPAZ_PHYS_WORLD_HPP
#define TOPAZ_PHYS_WORLD_HPP
#include "phys/body.hpp"
#include "phys/resolvers/generic.hpp"
#include <vector>

namespace tz::phys
{
    class World
    {
    public:
        using UniformForceID = std::size_t;
        World() = default;

        UniformForceID register_uniform_force(tz::Vec3 force_value);
        void unregister_uniform_force(UniformForceID force);
        void add_body(Body& body);
        void remove_body(Body& body);
        void add_resolver(IResolver& resolver);
        void remove_resolver(IResolver& resolver);
        void update(float delta_millis);
    private:
        void motion_integrate(float delta_millis);
        CollisionList detect_collisions();
        void resolve_collisions(CollisionList collisions, float delta_millis);
        tz::Vec3 fnet_uniform() const;

        std::vector<Body*> bodies;
        std::vector<IResolver*> resolvers;
        std::vector<tz::Vec3> uniform_forces;
    };
}

#endif // TOPAZ_PHYS_WORLD_HPP