#ifndef TOPAZ_PHYS_GENERIC_COLLIDER_HPP
#define TOPAZ_PHYS_GENERIC_COLLIDER_HPP
#include "core/vector.hpp"
#include <deque>

namespace tz::phys
{
    enum class ColliderType
    {
        Sphere,
        AABB
    };

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

    class ICollider
    {
    public:
        /// C++20 TODO: constexpr virtual
        virtual ColliderType get_type() const = 0;
        virtual CollisionPoint test_against(const ICollider& rhs) const = 0;

        void push(tz::Vec3 parent_position);
        void pop();
    protected:
        tz::Vec3 get_parent_position() const;
    private:
        std::deque<tz::Vec3> model_space_stack;
    };
}

#endif // TOPAZ_PHYS_GENERIC_COLLIDER_HPP