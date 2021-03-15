#include "phys/body.hpp"
#include "phys/motion.hpp"

namespace tz::phys
{
    typename World::UniformForceID World::register_uniform_force(tz::Vec3 force_value)
    {
        World::UniformForceID ret_id = this->uniform_forces.size();
        this->uniform_forces.push_back(force_value);
        return ret_id;
    }

    void World::unregister_uniform_force(World::UniformForceID force)
    {
        // We actually just nullify it.
        this->uniform_forces[force] = {};
    }

    void World::add_body(Body& body)
    {
        this->bodies.push_back(&body);
    }

    void World::remove_body(Body& body)
    {
        this->bodies.erase(std::remove(this->bodies.begin(), this->bodies.end(), &body), this->bodies.end());
    }

    void World::update(float delta_millis)
    {
        this->motion_integrate(delta_millis);
        this->detect_collisions();
    }

    void World::motion_integrate(float delta_millis)
    {
        for(Body* body_ptr : this->bodies)
        {
            topaz_assertf(body_ptr != nullptr, "tz::phys::World::motion_integrate(%g): A body was nullptr. This is not allowed.", delta_millis);
            tz::Vec3 fnet = fnet_uniform();
            body_ptr->force += fnet;
            motion::verlet_integrate(delta_millis, *body_ptr);
            body_ptr->force -= fnet;
        }
    }

    void World::detect_collisions()
    {
        for(Body* i : this->bodies)
        {
            tz::phys::ICollider* i_collider = i->collider.get();
            if(i_collider == nullptr)
            {
                continue;
            }
            i_collider->push(i->transform.position);
            for(Body* j : this->bodies)
            {
                if(i == j)
                {
                    continue;
                }
                tz::phys::ICollider* j_collider = j->collider.get();
                if(j_collider == nullptr)
                {
                    continue;
                }
                j_collider->push(j->transform.position);
                tz::phys::CollisionPoint collision = i_collider->test_against(*j_collider);
                i_collider->pop();
                j_collider->pop();
                if(collision.collides())
                {
                    // TODO: Sane Collision Response.
                    topaz_assert(false, "THIS IS THE CURRENT COLLISION RESPONSE. YES, THE COLLISION RESPONSE IS A BREAKPOINT. INCREDIBLE.");
                }
            }
        }
    }

    tz::Vec3 World::fnet_uniform() const
    {
        tz::Vec3 fnet{0.0f, 0.0f, 0.0f};
        for(const tz::Vec3& uniform_force : this->uniform_forces)
        {
            fnet += uniform_force;
        }
        return fnet;
    }
}