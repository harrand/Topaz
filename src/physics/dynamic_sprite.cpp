//
// Created by Harrand on 26/08/2018.
//

#include "dynamic_sprite.hpp"

DynamicSprite::DynamicSprite(float mass, Vector2F position_screenspace, float rotation, Vector2F scale, const Texture* texture, Vector2F velocity, float moment_of_inertia, float angular_velocity, std::initializer_list<Vector3F> forces): Sprite(position_screenspace, rotation, scale, texture), PhysicsObject(mass, {velocity, 0.0f}, moment_of_inertia, {0.0f, 0.0f, angular_velocity}, forces)
{
    for(Vector3F& force : this->forces)
        force.z = 0.0f;
}

void DynamicSprite::update(float delta_time)
{
    // This lambda performs verlet-integration. This is cheaper than forest-ruth but is slightly less accurate.
    auto verlet_integration = [&](float delta)
    {
        float half_delta = delta * 0.5f;
        this->position_screenspace += (this->velocity.xy() * half_delta);
        this->rotation += this->angular_velocity.z * half_delta;
        PhysicsObject::update(delta_time);
        this->position_screenspace += (this->velocity.xy() * half_delta);
        this->rotation += this->angular_velocity.z * half_delta;
    };
    // Perform forest-ruth motion integration. It utilises verlet-integration, so it more expensive but yields more accurate results.
    using namespace tz::utility::numeric;
    verlet_integration(delta_time * static_cast<float>(consts::forest_ruth_coefficient));
    verlet_integration(delta_time * static_cast<float>(consts::forest_ruth_complement));
    verlet_integration(delta_time * static_cast<float>(consts::forest_ruth_coefficient));
}

std::optional<AABB> DynamicSprite::get_boundary() const
{
    return tz::physics::bound_aabb(this->mesh) * tz::transform::model({position_screenspace.x, position_screenspace.y, 0.0f}, {0.0f, 0.0f, this->get_rotation()}, {this->scale, 0.0f});
}

void DynamicSprite::on_collision([[maybe_unused]] PhysicsObject& other){}