#ifndef DYNAMIC_OBJECT_HPP
#define DYNAMIC_OBJECT_HPP
#include "graphics/static_object.hpp"
#include "physics/physics_object.hpp"

class DynamicObject : public StaticObject, public PhysicsObject
{
public:
    DynamicObject(float mass, Transform transform, Asset asset, Vector3F velocity = {}, float moment_of_inertia = 1.0f, Vector3F angular_velocity = {}, std::initializer_list<Vector3F> forces = {});
    virtual void update(float delta_time) override;
    virtual std::optional<AABB> get_boundary() const override;
    virtual void on_collision(PhysicsObject& collided) override;

    friend class InstancedDynamicObject;
private:
    AABB bound_modelspace;
};

class InstancedDynamicObject : public DynamicObject
{
public:
    InstancedDynamicObject(const std::vector<DynamicObject>& objects);
    float get_mass() const;
    virtual void update(float delta_time) override;
    virtual void render(Shader& instanced_render_shader, const Camera& camera, const Vector2I& viewport_dimensions) const override;
    virtual std::optional<AABB> get_boundary() const override;
    virtual void on_collision(PhysicsObject& collided) override;
private:
    using DynamicObject::mass;
    using DynamicObject::get_boundary;
    using DynamicObject::on_collision;
    std::shared_ptr<InstancedMesh> instanced_mesh;
    std::vector<DynamicObject> objects;
};


#endif //DYNAMIC_OBJECT_HPP
