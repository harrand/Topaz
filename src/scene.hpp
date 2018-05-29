#ifndef TOPAZ_SCENE_HPP
#define TOPAZ_SCENE_HPP
#include "scene_object.hpp"

class Scene
{
public:
    Scene(const std::initializer_list<SceneObject> stack_objects = {}, const std::initializer_list<std::shared_ptr<SceneObject>> heap_objects = {});
    virtual void render(Shader& render_shader, const Camera& camera, const Vector2<int>& viewport_dimensions) const;
    std::vector<std::reference_wrapper<const SceneObject>> get_objects() const;
    void add_object(SceneObject scene_object);
    template<class Element, typename... Args>
    Element& emplace(Args&&... args);
    template<typename... Args>
    SceneObject& emplace_object(Args&&... args);
private:
    std::vector<SceneObject> stack_objects;
    std::vector<std::shared_ptr<SceneObject>> heap_objects;
};

#include "scene.inl"
#endif //TOPAZ_SCENE_HPP
