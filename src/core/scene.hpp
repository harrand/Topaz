#ifndef TOPAZ_SCENE_HPP
#define TOPAZ_SCENE_HPP
#include "physics/dynamic_object.hpp"
#include "physics/dynamic_sprite.hpp"

class Scene
{
public:
    Scene(const std::initializer_list<StaticObject>& stack_objects = {}, std::vector<std::unique_ptr<StaticObject>> heap_objects = {});

    virtual void render(Shader& render_shader, Shader* sprite_shader, const Camera& camera, const Vector2I& viewport_dimensions) const;
    virtual void update(float delta_time);
    std::vector<std::reference_wrapper<const StaticObject>> get_objects() const;
    std::vector<std::reference_wrapper<const Sprite>> get_sprites() const;
    AABB get_boundary() const;
    void add_object(StaticObject scene_object);
    template<class Element, typename... Args>
    Element& emplace(Args&&... args);
    template<typename... Args>
    StaticObject& emplace_object(Args&&... args);
    template<typename... Args>
    Sprite& emplace_sprite(Args&&... args);
    void remove_object(StaticObject& object);
    void remove_sprite(Sprite& sprite);
    std::optional<DirectionalLight> get_directional_light(std::size_t light_id) const;
    void set_directional_light(std::size_t light_id, DirectionalLight light);
    void add_directional_light(DirectionalLight light);
    std::optional<PointLight> get_point_light(std::size_t light_id) const;
    void set_point_light(std::size_t light_id, PointLight light);
    void add_point_light(PointLight light);
protected:
    std::vector<std::reference_wrapper<const StaticObject>> get_static_objects() const;
    std::vector<std::reference_wrapper<const DynamicObject>> get_dynamic_objects() const;
    std::vector<std::reference_wrapper<StaticObject>> get_mutable_static_objects();
    std::vector<std::reference_wrapper<DynamicObject>> get_mutable_dynamic_objects();
    std::vector<std::reference_wrapper<Sprite>> get_mutable_sprites();
    std::vector<std::reference_wrapper<DynamicSprite>> get_mutable_dynamic_sprites();
protected:
    std::vector<StaticObject> stack_objects;
    std::vector<std::unique_ptr<StaticObject>> heap_objects;
    std::vector<Sprite> stack_sprites;
    std::vector<std::unique_ptr<Sprite>> heap_sprites;
    std::vector<DirectionalLight> directional_lights;
    std::vector<PointLight> point_lights;
    private:
    void erase_object(StaticObject* to_delete);
    void erase_sprite(Sprite* to_delete);
    void handle_deletions();
    std::vector<StaticObject*> objects_to_delete;
    std::vector<Sprite*> sprites_to_delete;
};

#include "scene.inl"
#endif //TOPAZ_SCENE_HPP
