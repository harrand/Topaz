//
// Created by Harrand on 26/08/2018.
//

#ifndef TOPAZ_DYNAMIC_SPRITE_HPP
#define TOPAZ_DYNAMIC_SPRITE_HPP
#include "graphics/sprite.hpp"
#include "physics/physics_object.hpp"

class DynamicSprite : public Sprite, public PhysicsObject
{
public:
    DynamicSprite(float mass, Vector2F position_screenspace, float rotation, Vector2F scale, const Texture* texture, Vector2F velocity = {}, float moment_of_inertia = 1.0f, float angular_velocity = 0.0f, std::initializer_list<Vector3F> forces = {});
    virtual void update(float delta_time) override;
    virtual std::optional<AABB> get_boundary() const override;
    virtual void on_collision(PhysicsObject& collided) override;
};


#endif //TOPAZ_DYNAMIC_SPRITE_HPP
