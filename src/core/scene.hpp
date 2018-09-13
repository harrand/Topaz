#ifndef TOPAZ_SCENE_HPP
#define TOPAZ_SCENE_HPP
#include "physics/dynamic_object.hpp"
#include "physics/dynamic_sprite.hpp"
#include "physics/physics.hpp"
#include <map>

class Scene
{
public:
    Scene(const std::initializer_list<StaticObject>& stack_objects = {}, std::vector<std::unique_ptr<StaticObject>> heap_objects = {});

    virtual void render(Shader* render_shader, Shader* sprite_shader, const Camera& camera, const Vector2I& viewport_dimensions) const;
    virtual void update(float delta_time);
    std::size_t get_number_of_static_objects() const;
    std::size_t get_number_of_sprites() const;
    std::size_t get_number_of_elements() const;
    std::vector<std::reference_wrapper<const StaticObject>> get_static_objects() const;
    std::vector<std::reference_wrapper<const Sprite>> get_sprites() const;
    AABB get_boundary() const;
    template<class Element, typename... Args>
    Element& emplace(Args&&... args);
    void add_object(StaticObject scene_object);
    template<typename... Args>
    StaticObject& emplace_object(Args&&... args);
    void remove_object(StaticObject& object);
    void add_sprite(Sprite sprite);
    template<typename... Args>
    Sprite& emplace_sprite(Args&&... args);
    void remove_sprite(Sprite& sprite);
    std::optional<DirectionalLight> get_directional_light(std::size_t light_id) const;
    void set_directional_light(std::size_t light_id, DirectionalLight light);
    void add_directional_light(DirectionalLight light);
    std::optional<PointLight> get_point_light(std::size_t light_id) const;
    void set_point_light(std::size_t light_id, PointLight light);
    void add_point_light(PointLight light);
protected:
    std::vector<std::reference_wrapper<const DynamicObject>> get_dynamic_objects() const;
    std::vector<std::reference_wrapper<StaticObject>> get_mutable_static_objects();
    std::vector<std::reference_wrapper<DynamicObject>> get_mutable_dynamic_objects();
    std::vector<std::reference_wrapper<const DynamicSprite>> get_dynamic_sprites() const;
    std::vector<std::reference_wrapper<Sprite>> get_mutable_sprites();
    std::vector<std::reference_wrapper<DynamicSprite>> get_mutable_dynamic_sprites();
    std::multimap<float, std::reference_wrapper<DynamicObject>> get_mutable_dynamic_objects_sorted_by_variance_axis();
    std::multimap<float, std::reference_wrapper<DynamicSprite>> get_mutable_dynamic_sprites_sorted_by_variance_axis();
    tz::physics::Axis3D get_highest_variance_axis_objects() const;
    tz::physics::Axis2D get_highest_variance_axis_sprites() const;
    void erase_object(StaticObject* to_delete);
    void erase_sprite(Sprite* to_delete);
    void handle_deletions();

    std::vector<StaticObject> stack_objects;
    std::vector<std::unique_ptr<StaticObject>> heap_objects;
    std::vector<Sprite> stack_sprites;
    std::vector<std::unique_ptr<Sprite>> heap_sprites;
    std::vector<DirectionalLight> directional_lights;
    std::vector<PointLight> point_lights;
    std::vector<StaticObject*> objects_to_delete;
    std::vector<Sprite*> sprites_to_delete;
};

#include "scene.inl"
#endif //TOPAZ_SCENE_HPP
