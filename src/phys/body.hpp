#ifndef TOPAZ_PHYS_BODY_HPP
#define TOPAZ_PHYS_BODY_HPP
#include "core/vector.hpp"
#include "phys/colliders/generic.hpp"
#include "gl/transform.hpp"
#include <vector>

namespace tz::phys
{
    struct Body
    {
        tz::gl::Transform& transform;
        tz::Vec3 velocity;
        tz::Vec3 force;
        float mass;

        std::unique_ptr<ICollider> collider = nullptr;
    };
}

#endif // TOPAZ_PHYS_BODY_HPP