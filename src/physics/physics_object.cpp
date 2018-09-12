#include "physics/physics_object.hpp"

PhysicsObject::PhysicsObject(float mass, Vector3F velocity, float moment_of_inertia, Vector3F angular_velocity, std::initializer_list<Vector3F> forces, std::initializer_list<Vector3F> torques): mass(mass), velocity(velocity), moment_of_inertia(moment_of_inertia), angular_velocity(angular_velocity), forces(forces), torques(torques), colliding_with(){}

Vector3F PhysicsObject::net_force() const
{
    return std::accumulate(this->forces.begin(), this->forces.end(), Vector3F{});
}

Vector3F PhysicsObject::net_torque() const
{
    return std::accumulate(this->torques.begin(), this->torques.end(), Vector3F{});
}

Vector3F PhysicsObject::get_acceleration() const
{
    return this->net_force() / this->mass;
}

Vector3F PhysicsObject::get_angular_acceleration() const
{
    return this->net_torque() / this->moment_of_inertia;
}

void PhysicsObject::update(float delta_time)
{
    this->velocity += (this->get_acceleration() * delta_time);
    this->angular_velocity += (this->get_angular_acceleration() * delta_time);
}

void PhysicsObject::add_force(Vector3F force)
{
    this->forces.push_back(force);
}

void PhysicsObject::add_torque(Vector3F torque)
{
    this->torques.push_back(torque);
}

void PhysicsObject::clear_forces()
{
    this->forces.clear();
}

void PhysicsObject::clear_torques()
{
    this->torques.clear();
}

void PhysicsObject::handle_collisions(const std::vector<std::reference_wrapper<PhysicsObject>>& physics_objects)
{
    for(auto ref : physics_objects)
    {
        auto& physics_object = ref.get();
        if(this == &physics_object)
            continue;
        using OptAABB = std::optional<AABB>;
        OptAABB this_bound = this->get_boundary();
        OptAABB other_bound = physics_object.get_boundary();
        if(this_bound.has_value() && other_bound.has_value())
        {
            if(this_bound->intersects(other_bound.value()) && std::find(this->colliding_with.begin(), this->colliding_with.end(), &physics_object) == this->colliding_with.end())
            {
                this->colliding_with.push_back(&physics_object);
                this->on_collision(physics_object);
            }
            else if(!this_bound->intersects(other_bound.value()) && std::find(this->colliding_with.begin(), this->colliding_with.end(), &physics_object) != this->colliding_with.end())
                this->colliding_with.erase(std::remove(this->colliding_with.begin(), this->colliding_with.end(), &physics_object), this->colliding_with.end());
        }
    }
}

void PhysicsObject::handle_collisions_sort_and_sweep(tz::physics::Axis2D highest_variance_axis, const std::multimap<float, std::reference_wrapper<PhysicsObject>>& physics_objects_sorted)
{
    using namespace tz::physics;
    switch(highest_variance_axis)
    {
        case Axis2D::X:
        default:
            this->handle_collisions_sort_and_sweep(Axis3D::X, physics_objects_sorted);
            break;
        case Axis2D::Y:
            this->handle_collisions_sort_and_sweep(Axis3D::Y, physics_objects_sorted);
            break;
    }
}

void PhysicsObject::handle_collisions_sort_and_sweep(tz::physics::Axis3D highest_variance_axis, const std::multimap<float, std::reference_wrapper<PhysicsObject>>& physics_objects_sorted)
{
    if(physics_objects_sorted.empty())
        return;
    std::vector<std::reference_wrapper<PhysicsObject>> possible_colliders;
    // assume highest variance is X for now.
    /*
        std::multimap helper:
        lower_bound

        returns an iterator to the first element not less than the given key
        (public member function)
        upper_bound

        returns an iterator to the first element greater than the given key
        (public member function)
    */
    AABB this_boundary = this->get_boundary().value();
    std::multimap<float, std::reference_wrapper<PhysicsObject>>::const_iterator next_possible_collidee_iter = physics_objects_sorted.lower_bound(this_boundary.get_minimum().x);
    float current_min_value;// = this_boundary.get_minimum().x;
    using namespace tz::physics;
    switch(highest_variance_axis)
    {
        case Axis3D::X:
        default:
            current_min_value = this_boundary.get_minimum().x;
            break;
        case Axis3D::Y:
            current_min_value = this_boundary.get_minimum().y;
            break;
        case Axis3D::Z:
            current_min_value = this_boundary.get_minimum().z;
            break;
    }
    std::ptrdiff_t offset = 0;
    while(next_possible_collidee_iter != physics_objects_sorted.cend())
    {
        // lol good luck understanding this.
        /**
         * ok so. upper bound skips objects with the same value so we cant do that.
         * BUT, lower bound just keeps giving us the same object so we need to use an offset.
         * so between lower bound (probably this) and upper bound (the thing with next highest value), we iterate slowly via the offset ptrdiff.
         * then when we get to a point where the ptrdiff brings us to the upper bound, we just take that upper bound as the new, reset the offset ptrdiff and continue.
         * THIS IS UNTESTED. MAY GO BAD BUT I'VE BEEN HERE FOR 3 HOURS AND IT WORKS TO AN EXTENT. I'M REALLY SORRY IF YOU EVER HAVE TO COME BACK TO THIS HELLHOLE.
         * you know when you see those cringe block comments from neckbeards? this is mine.
         */
        //next_possible_collidee_iter = physics_objects_sorted.upper_bound(current_min_value);
        std::pair iter_pair = physics_objects_sorted.equal_range(current_min_value);
        if(iter_pair.first == physics_objects_sorted.end())
            break;
        auto iter_begin = std::next(iter_pair.first, ++offset);
        if(iter_begin == physics_objects_sorted.end())
            break;
        next_possible_collidee_iter = iter_begin;
        if(iter_begin == iter_pair.second)
            offset = 0;
        PhysicsObject& next_possible_collidee = next_possible_collidee_iter->second.get();
        bool should_break = false;
        AABB next_possible_collidee_bound = next_possible_collidee.get_boundary().value();
        auto ensure_not_colliding = [&]()
        {
            should_break = true;
            if(std::find(this->colliding_with.begin(), this->colliding_with.end(), &next_possible_collidee) != this->colliding_with.end())
                this->colliding_with.erase(std::remove(this->colliding_with.begin(), this->colliding_with.end(), &next_possible_collidee));
        };
        switch(highest_variance_axis)
        {
            case Axis3D::X:
            default:
                current_min_value = next_possible_collidee_bound.get_minimum().x;
                if(current_min_value <= this_boundary.get_maximum().x)
                    possible_colliders.push_back(std::ref(next_possible_collidee));
                else
                    ensure_not_colliding();
                break;
            case Axis3D::Y:
                current_min_value = next_possible_collidee_bound.get_minimum().y;
                if(current_min_value <= this_boundary.get_maximum().y)
                    possible_colliders.push_back(std::ref(next_possible_collidee));
                else
                    ensure_not_colliding();
                break;
            case Axis3D::Z:
                current_min_value = next_possible_collidee_bound.get_minimum().z;
                if(current_min_value <= this_boundary.get_maximum().z)
                    possible_colliders.push_back(std::ref(next_possible_collidee));
                else
                    ensure_not_colliding();
                break;
        }
        if(should_break)
            break;
    }
    this->handle_collisions(possible_colliders);
}