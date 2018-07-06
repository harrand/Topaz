#ifndef DYNAMIC_OBJECT_HPP
#define DYNAMIC_OBJECT_HPP
#include "graphics/static_object.hpp"
#include "physics/physics_object.hpp"

class DynamicObject : public StaticObject, public PhysicsObject
{
public:
    DynamicObject(float mass, Transform transform, Asset asset, Vector3F velocity = {}, float moment_of_inertia = 1.0f, Vector3F angular_velocity = {}, std::initializer_list<Vector3F> forces = {});
    virtual void update(float delta_time) override;
    virtual AABB get_boundary() const override;
    virtual void on_collision(PhysicsObject& collided) override;
private:
    AABB bound_modelspace;
};


#endif //DYNAMIC_OBJECT_HPP
