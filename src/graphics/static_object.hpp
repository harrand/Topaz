#ifndef TOPAZ_SCENE_OBJECT_HPP
#define TOPAZ_SCENE_OBJECT_HPP

#include "graphics/mesh.hpp"
#include "graphics/asset.hpp"
#include "data/transform.hpp"
#include "physics/physics_object.hpp"
#include "graphics/render_pass.hpp"

/**
 * StaticObjects do NOT own the assets that they use.
 * The assets provided MUST have a lifetime greater than or equal to the SceneObject.
 */
class StaticObject
{
public:
    StaticObject(Transform transform, Asset asset);
    const Asset& get_asset() const;
    virtual std::optional<AABB> get_boundary() const;
    virtual void render(RenderPass render_pass) const;

    Transform transform;

    bool operator==(const StaticObject& rhs) const;
    friend class InstancedStaticObject;
protected:
    Asset asset;
};

class InstancedStaticObject : public StaticObject
{
public:
    InstancedStaticObject(const std::vector<StaticObject>& objects);
    const InstancedMesh& get_instanced_mesh() const;
    virtual void render(RenderPass render_pass) const override;
private:
    std::shared_ptr<InstancedMesh> instanced_mesh;
};

#endif //TOPAZ_SCENE_OBJECT_HPP
