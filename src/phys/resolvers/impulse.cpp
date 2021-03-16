#include "phys/resolvers/impulse.hpp"
#include "phys/body.hpp"
#include <utility>

namespace tz::phys
{
    void ImpulseResolver::solve_all(CollisionList& collisions, [[maybe_unused]] float delta_millis)
    {
        for(CollisionSituation& situation : collisions)
        {
            constexpr bool is_kinematic = true;
            tz::Vec3 a_vel = situation.a.velocity;
            tz::Vec3 b_vel = situation.b.velocity;
            tz::Vec3 r_vel = b_vel - a_vel;
            float normal_speed = r_vel.dot(situation.point.normal());

            float inv_mass_a = 1.0f / situation.a.mass;
            float inv_mass_b = 1.0f / situation.b.mass;

            // Impulse section

            if(normal_speed >= 0)
            {
                continue;
            }

            float e = situation.a.restitution * situation.b.restitution;

            float j = -(1.0f + e) * normal_speed / (inv_mass_a + inv_mass_b);

            tz::Vec3 impulse = situation.point.normal() * j;
            if(is_kinematic)
            {
                a_vel -= impulse * inv_mass_a;
                b_vel += impulse * inv_mass_b;
            }

            // Friction section
            r_vel = b_vel - a_vel;
            normal_speed = r_vel.dot(situation.point.normal());
            tz::Vec3 tangent = (r_vel - situation.point.normal() * normal_speed).normalised();
            float f_vel = r_vel.dot(tangent);

            float a_static = situation.a.static_friction;
            float b_static = situation.b.static_friction;
            float a_dynamic = situation.a.dynamic_friction;
            float b_dynamic = situation.b.dynamic_friction;

            float mu = std::hypot(a_static, b_static);
            float f = -f_vel / (inv_mass_a + inv_mass_b);

            tz::Vec3 friction;
            if(std::fabs(f) < j * mu)
            {
                friction = tangent * f;
            }
            else
            {
                mu = std::hypot(a_dynamic, b_dynamic);
                friction = tangent * -j * mu;
            }

            if(is_kinematic)
            {
                situation.a.velocity = a_vel - friction * inv_mass_a;
                situation.b.velocity = b_vel + friction * inv_mass_b;
            }
        }
    }
}