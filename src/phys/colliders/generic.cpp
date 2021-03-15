#include "phys/colliders/generic.hpp"
#include <numeric>

namespace tz::phys
{
    CollisionPoint::CollisionPoint(tz::Vec3 a, tz::Vec3 b, bool has_collision): a(a), b(b), has_collision(has_collision){}
 
    tz::Vec3 CollisionPoint::normal() const
    {
        return this->b - this->a;
    }

    float CollisionPoint::depth() const
    {
        return this->normal().length();
    }

    bool CollisionPoint::collides() const
    {
        return this->has_collision;
    }

    void ICollider::push(tz::Vec3 parent_position)
    {
        this->model_space_stack.push_back(parent_position);
    }

    void ICollider::pop()
    {
        topaz_assert(!this->model_space_stack.empty(), "tz::phys::ICollider::pop(): Parent body stack was empty before popping.");
        this->model_space_stack.pop_front();
    }

    tz::Vec3 ICollider::get_parent_position() const
    {
        return std::accumulate(this->model_space_stack.begin(), this->model_space_stack.end(), tz::Vec3{0.0f, 0.0f, 0.0f});
    }
}