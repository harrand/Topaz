//
// Created by Harrand on 26/08/2018.
//

#ifndef TOPAZ_DYNAMIC_SPRITE_HPP
#define TOPAZ_DYNAMIC_SPRITE_HPP
#include "graphics/sprite.hpp"
#include "physics/physics_object.hpp"

/**
 * Like a normal Sprite, but has motion integration and collision detection.
 */
class DynamicSprite : public Sprite, public PhysicsObject
{
public:
	/**
	 * Construct a DynamicSprite with the given properties.
	 * @param mass - Mass of the sprite, in kilograms
	 * @param position_screenspace - Position of the sprite on the screen, in pixels
	 * @param rotation - Rotation of the sprite, in radians
	 * @param scale - Scale of the sprite
	 * @param texture - Texture to bind to the sprite plane
	 * @param velocity - Initial velocity of the sprite, in world-units per second
	 * @param moment_of_inertia - Moment of inertia of the sprite, in kilogram world-units squared
	 * @param angular_velocity - Initial angular velocity of the sprite, in radians per second
	 * @param forces - Container of forces applied to the sprite, if any
	 */
	DynamicSprite(float mass, Vector2F position_screenspace, float rotation, Vector2F scale, const Texture* texture, Vector2F velocity = {}, float moment_of_inertia = 1.0f, float angular_velocity = 0.0f, std::initializer_list<Vector3F> forces = {});
	/**
	 * Perform an update. This should be invoked once per tick.
	 * @param delta_time - Time passed since the previous tick, in seconds
	 */
	virtual void update(float delta_time) override;
	/**
	 * Retrieve an AABB bounding this DynamicSprite.
	 * @return - AABB bounding this sprite
	 */
	virtual std::optional<AABB> get_boundary() const override;
	/**
	 * Callback function, invoked when a collision is detected between this sprite and another PhysicsObject.
	 * @param collided - The PhysicsObject that collided with this sprite
	 */
	virtual void on_collision(PhysicsObject& collided) override;
};

#endif //TOPAZ_DYNAMIC_SPRITE_HPP
