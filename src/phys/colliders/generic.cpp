#include "phys/colliders/generic.hpp"
#include <numeric>

namespace tz::phys
{
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