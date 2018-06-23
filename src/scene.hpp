#ifndef TOPAZ_SCENE_HPP
#define TOPAZ_SCENE_HPP
#include "object.hpp"

class Scene
{
public:
    Scene(const std::initializer_list<StaticObject>& stack_objects = {}, const std::initializer_list<std::shared_ptr<StaticObject>>& heap_objects = {});
    virtual void render(Shader& render_shader, const Camera& camera, const Vector2<int>& viewport_dimensions) const;
    virtual void update(float delta_time);
    std::vector<std::reference_wrapper<const StaticObject>> get_objects() const;
    void add_object(StaticObject scene_object);
    template<class Element, typename... Args>
    Element& emplace(Args&&... args);
    template<typename... Args>
    StaticObject& emplace_object(Args&&... args);
protected:
    std::vector<std::reference_wrapper<StaticObject>> get_static_objects();
    std::vector<std::reference_wrapper<DynamicObject>> get_dynamic_objects();
private:
    std::vector<StaticObject> stack_objects;
    std::vector<std::shared_ptr<StaticObject>> heap_objects;
};

#include "scene.inl"
#endif //TOPAZ_SCENE_HPP
