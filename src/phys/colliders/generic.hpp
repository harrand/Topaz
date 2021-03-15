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
        /**
         * Retrieve the type of collider
         * @return ColliderType corresponding to this collider instance.
         */
        virtual ColliderType get_type() const = 0;
        /**
         * Test this collider against another existing collider.
         * Precondition: The collider types must be compatible. If not, this will assert and invoke UB.
         * Precondition: Both colliders must have been push()ed so that they are both in world-space. Otherwise this will yield unexpected results without asserting.
         */
        virtual CollisionPoint test_against(const ICollider& rhs) const = 0;
        /**
         * Colliders belong to an existing Body. Base collider properties are in local-space. Chances are you will want to test colliders in world-space, not local space.
         * To do that, use push(position) such that the origin (0, 0, 0) in local-space is transformed by that position. You can push multiple times, and the new origin will be the total of all pushed positions, allowing for a hierarchical structure.
         * @param parent_position The position of the body, in world-space.
         */
        void push(tz::Vec3 parent_position);
        /**
         * It is necessary to test colliders directly after popping the body's position. It is feasible that the body position has since moved. For that reason, the collider must be popped and re-pushed before the next collison check to update the world-space location.
         * Precondition: A body position was pushed earlier that has not yet been popped. Otherwise, this will assert and invoke UB.
         */
        void pop();
    protected:
        /**
         * Retrieve the position of the parent body, in world-space. Colliders will likely need this as their properties will be in local-space.
         * @return Position of the parent body, in world-space.
         */
        tz::Vec3 get_parent_position() const;
    private:
        std::deque<tz::Vec3> model_space_stack;
    };
}

#endif // TOPAZ_PHYS_GENERIC_COLLIDER_HPP