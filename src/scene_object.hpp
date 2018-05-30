#ifndef TOPAZ_SCENE_OBJECT_HPP
#define TOPAZ_SCENE_OBJECT_HPP

#include <graphics/mesh.hpp>
#include "asset.hpp"
#include "camera.hpp"
#include "transform.hpp"

/**
 * SceneObjects do NOT own the assets that they use.
 * The assets provided MUST have a lifetime greater than or equal to the SceneObject.
 */
class SceneObject
{
public:
    SceneObject(Transform transform, Asset asset);
    virtual void render(Shader& render_shader, const Camera& camera, const Vector2<int>& viewport_dimensions) const;
    Transform transform;
private:
    Asset asset;
};

#endif //TOPAZ_SCENE_OBJECT_HPP
