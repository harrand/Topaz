#include "phys/motion.hpp"
#include "phys/body.hpp"

namespace tz::phys::motion
{
    void verlet_integrate(float delta_millis, Body& body)
    {
        float delta_seconds = delta_millis / 1000.0f;
        float half_delta = delta_seconds * 0.5f;
        // (1) Half-update position, (2) update velocity, and then (3) the other half of position.
        // (1)
        body.transform.position += (body.velocity * half_delta);
        // (2)
        // f = ma
        // a = f/m
        // v = v0 + at
        body.velocity += (body.force / body.mass) * delta_seconds;

        // (3)
        body.transform.position += (body.velocity * half_delta);
    }
}