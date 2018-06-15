#ifndef TOPAZ_SCENE_OBJECT_HPP
#define TOPAZ_SCENE_OBJECT_HPP

#include <graphics/mesh.hpp>
#include "asset.hpp"
#include "camera.hpp"
#include "transform.hpp"
#include "physics/physics_object.hpp"

/**
 * StaticObjects do NOT own the assets that they use.
 * The assets provided MUST have a lifetime greater than or equal to the SceneObject.
 */
class StaticObject
{
public:
    StaticObject(Transform transform, Asset asset);
    const Asset& get_asset() const;
    virtual void render(Shader& render_shader, const Camera& camera, const Vector2<int>& viewport_dimensions) const;
    Transform transform;
private:
    Asset asset;
};

class DynamicObject : public StaticObject, public PhysicsObject
{
public:
    DynamicObject(float mass, Transform transform, Asset asset, Vector3F velocity = {}, Vector3F angular_velocity = {}, std::initializer_list<Vector3F> forces = {});
    virtual void update(float delta_time) override;
};

#endif //TOPAZ_SCENE_OBJECT_HPP
