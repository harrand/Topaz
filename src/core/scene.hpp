#ifndef TOPAZ_SCENE_HPP
#define TOPAZ_SCENE_HPP
#include "physics/dynamic_object.hpp"

class Scene
{
public:
    Scene(const std::initializer_list<StaticObject>& stack_objects = {}, std::vector<std::unique_ptr<StaticObject>> heap_objects = {});
    virtual void render(Shader& render_shader, const Camera& camera, const Vector2I& viewport_dimensions) const;
    virtual void update(float delta_time);
    std::vector<std::reference_wrapper<const StaticObject>> get_objects() const;
    AABB get_boundary() const;
    void add_object(StaticObject scene_object);
    template<class Element, typename... Args>
    Element& emplace(Args&&... args);
    template<typename... Args>
    StaticObject& emplace_object(Args&&... args);
    std::optional<DirectionalLight> get_directional_light(std::size_t light_id) const;
    void set_directional_light(std::size_t light_id, DirectionalLight light);
    void add_directional_light(DirectionalLight light);
    std::optional<PointLight> get_point_light(std::size_t light_id) const;
    void set_point_light(std::size_t light_id, PointLight light);
    void add_point_light(PointLight light);
protected:
    std::vector<std::reference_wrapper<StaticObject>> get_static_objects();
    std::vector<std::reference_wrapper<DynamicObject>> get_dynamic_objects();
private:
    std::vector<StaticObject> stack_objects;
    std::vector<std::unique_ptr<StaticObject>> heap_objects;
    std::vector<DirectionalLight> directional_lights;
    std::vector<PointLight> point_lights;
};

#include "scene.inl"
#endif //TOPAZ_SCENE_HPP
