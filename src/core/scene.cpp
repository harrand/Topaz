#include "core/scene.hpp"
#include "physics/physics.hpp"

Scene::Scene(const std::initializer_list<StaticObject>& stack_objects, std::vector<std::unique_ptr<StaticObject>> heap_objects): stack_objects(stack_objects), heap_objects(std::move(heap_objects)), stack_sprites{}, heap_sprites{}, directional_lights{}, point_lights{}, objects_to_delete{}, sprites_to_delete{}{}

void Scene::render(Shader& render_shader, Shader* sprite_shader, const Camera& camera, const Vector2I& viewport_dimensions) const
{
    Frustum camera_frustum(camera, viewport_dimensions.x / viewport_dimensions.y);
    auto render_if_visible = [&](const StaticObject& object){AABB object_box = tz::physics::bound_aabb(*(object.get_asset().mesh)); if(camera_frustum.contains(object_box * object.transform.model()) || tz::graphics::is_instanced(object.get_asset().mesh)) object.render(render_shader, camera, viewport_dimensions);};
    for(const auto& static_object : this->get_static_objects())
    {
        if(std::find(this->objects_to_delete.begin(), this->objects_to_delete.end(), &static_object.get()) != this->objects_to_delete.end())
            continue;
        render_if_visible(static_object.get());
    }
    if(sprite_shader != nullptr)
    {
        for(const auto& sprite : this->get_sprites())
        {
            if(std::find(this->sprites_to_delete.begin(), this->sprites_to_delete.end(), &sprite.get()) != this->sprites_to_delete.end())
                continue;
            sprite.get().render(*sprite_shader, viewport_dimensions);
        }
    }
    for(std::size_t i = 0; i < this->directional_lights.size(); i++)
    {
        render_shader.bind();
        render_shader.set_uniform<DirectionalLight>(std::string("directional_lights[") + std::to_string(i) + "]", this->directional_lights[i]);
        render_shader.update();
    }
    for(std::size_t i = 0; i < this->point_lights.size(); i++)
    {
        render_shader.bind();
        render_shader.set_uniform<PointLight>(std::string("point_lights[") + std::to_string(i) + "]", this->point_lights[i]);
        render_shader.update();
    }
}

void Scene::update(float delta_time)
{
    for(std::reference_wrapper<DynamicObject> dynamic_ref : this->get_mutable_dynamic_objects())
        dynamic_ref.get().update(delta_time);
    for(std::reference_wrapper<DynamicSprite> dynamic_sprite_ref : this->get_mutable_dynamic_sprites())
        dynamic_sprite_ref.get().update(delta_time);
    std::vector<std::reference_wrapper<PhysicsObject>> physics_objects;
    std::vector<std::reference_wrapper<PhysicsObject>> physics_sprites;
    for(auto& object : this->get_mutable_static_objects())
    {
        if(std::find(this->objects_to_delete.begin(), this->objects_to_delete.end(), &object.get()) != this->objects_to_delete.end())
            continue;
        auto physics_component = dynamic_cast<PhysicsObject*>(&object.get());
        if(physics_component != nullptr)
            physics_objects.push_back(std::ref(*physics_component));
    }
    for(auto& sprite : this->get_mutable_sprites())
    {
        auto physics_component = dynamic_cast<PhysicsObject*>(&sprite.get());
        if(physics_component != nullptr)
            physics_sprites.push_back(std::ref(*physics_component));
    }
    for(auto& physics_ref : physics_objects)
        physics_ref.get().handle_collisions(physics_objects);
    for(auto& physics_sprite_ref : physics_sprites)
        physics_sprite_ref.get().handle_collisions(physics_sprites);
    this->handle_deletions();
}

std::vector<std::reference_wrapper<const StaticObject>> Scene::get_objects() const
{
    std::vector<std::reference_wrapper<const StaticObject>> object_crefs;
    for(const StaticObject& object_cref : this->stack_objects)
        object_crefs.push_back(std::cref(object_cref));
    for(const std::unique_ptr<StaticObject>& object_ptr : this->heap_objects)
        object_crefs.push_back(std::cref(*object_ptr));
    return object_crefs;
}

std::vector<std::reference_wrapper<const Sprite>> Scene::get_sprites() const
{
    std::vector<std::reference_wrapper<const Sprite>> sprite_crefs;
    for(const Sprite& sprite_cref : this->stack_sprites)
        sprite_crefs.push_back(std::cref(sprite_cref));
    for(const std::unique_ptr<Sprite>& sprite_ptr : this->heap_sprites)
        sprite_crefs.push_back(std::cref(*sprite_ptr));
    return sprite_crefs;
}

AABB Scene::get_boundary() const
{
    auto objects = this->get_objects();
    if(objects.size() == 0)
        return {{}, {}};
    Vector3F min = objects.front().get().transform.position, max = objects.front().get().transform.position;
    for(const StaticObject& object : objects)
    {
        auto boundary_optional = object.get_boundary();
        if(!boundary_optional.has_value())
            continue;
        const AABB& boundary = boundary_optional.value();
        min.x = std::min(min.x, boundary.get_minimum().x);
        min.y = std::min(min.y, boundary.get_minimum().y);
        min.z = std::min(min.z, boundary.get_minimum().z);

        max.x = std::max(max.x, boundary.get_maximum().x);
        max.y = std::max(max.y, boundary.get_maximum().y);
        max.z = std::max(max.z, boundary.get_maximum().z);
    }
    return {min, max};
}

void Scene::add_object(StaticObject scene_object)
{
    this->stack_objects.push_back(scene_object);
}

// add remove sprite and remove object here

void Scene::remove_object(StaticObject& object)
{
    this->objects_to_delete.push_back(&object);
}

void Scene::remove_sprite(Sprite& sprite)
{
    this->sprites_to_delete.push_back(&sprite);
}

std::optional<DirectionalLight> Scene::get_directional_light(std::size_t light_id) const
{
    try
    {
        return {this->directional_lights.at(light_id)};
    }
    catch(const std::out_of_range& range_exception)
    {
        return {};
    }
}

void Scene::set_directional_light(std::size_t light_id, DirectionalLight light)
{
    if(light_id >= this->directional_lights.size())
        this->directional_lights.resize(light_id + 1);
    this->directional_lights.at(light_id) = light;
}

void Scene::add_directional_light(DirectionalLight light)
{
    this->directional_lights.push_back(std::move(light));
}

std::optional<PointLight> Scene::get_point_light(std::size_t light_id) const
{
    try
    {
        return {this->point_lights.at(light_id)};
    }
    catch(const std::out_of_range& range_exception)
    {
        return {};
    }
}

void Scene::set_point_light(std::size_t light_id, PointLight light)
{
    if(light_id >= this->point_lights.size())
        this->point_lights.resize(light_id + 1);
    this->point_lights.at(light_id) = light;
}

void Scene::add_point_light(PointLight light)
{
    this->point_lights.push_back(std::move(light));
}

std::vector<std::reference_wrapper<const StaticObject>> Scene::get_static_objects() const
{
    std::vector<std::reference_wrapper<const StaticObject>> object_crefs;
    for(const StaticObject& object_ref : this->stack_objects)
        object_crefs.push_back(std::cref(object_ref));
    for(const std::unique_ptr<StaticObject>& object_ptr : this->heap_objects)
        object_crefs.push_back(std::cref(*object_ptr));
    return object_crefs;
}

std::vector<std::reference_wrapper<const DynamicObject>> Scene::get_dynamic_objects() const
{
    std::vector<std::reference_wrapper<const DynamicObject>> object_crefs;
    for(std::reference_wrapper<const StaticObject> static_ref : this->get_static_objects())
    {
        const DynamicObject* dynamic_ref = dynamic_cast<const DynamicObject*>(&static_ref.get());
        if(dynamic_ref != nullptr)
            object_crefs.push_back(std::cref(*dynamic_ref));
    }
    return object_crefs;
}

std::vector<std::reference_wrapper<StaticObject>> Scene::get_mutable_static_objects()
{
    std::vector<std::reference_wrapper<StaticObject>> object_refs;
    for(StaticObject& object_ref : this->stack_objects)
        object_refs.push_back(std::ref(object_ref));
    for(std::unique_ptr<StaticObject>& object_ptr : this->heap_objects)
        object_refs.push_back(std::ref(*object_ptr));
    return object_refs;
}

std::vector<std::reference_wrapper<DynamicObject>> Scene::get_mutable_dynamic_objects()
{
    std::vector<std::reference_wrapper<DynamicObject>> object_refs;
    for(std::reference_wrapper<StaticObject> static_ref : this->get_mutable_static_objects())
    {
        DynamicObject* dynamic_ref = dynamic_cast<DynamicObject*>(&static_ref.get());
        if(dynamic_ref != nullptr)
            object_refs.push_back(std::ref(*dynamic_ref));
    }
    return object_refs;
}

std::vector<std::reference_wrapper<Sprite>> Scene::get_mutable_sprites()
{
    std::vector<std::reference_wrapper<Sprite>> sprite_refs;
    for(Sprite& sprite_cref : this->stack_sprites)
        sprite_refs.push_back(std::ref(sprite_cref));
    for(std::unique_ptr<Sprite>& sprite_ptr : this->heap_sprites)
        sprite_refs.push_back(std::ref(*sprite_ptr));
    return sprite_refs;
}

std::vector<std::reference_wrapper<DynamicSprite>> Scene::get_mutable_dynamic_sprites()
{
    std::vector<std::reference_wrapper<DynamicSprite>> dyn_sprite_refs;
    for(std::reference_wrapper<Sprite> static_ref : this->get_mutable_sprites())
    {
        DynamicSprite* dynamic_component = dynamic_cast<DynamicSprite*>(&static_ref.get());
        if(dynamic_component != nullptr)
            dyn_sprite_refs.push_back(std::ref(*dynamic_component));
    }
    return dyn_sprite_refs;
}

void Scene::erase_object(StaticObject* to_delete)
{
    auto stack_iterator = std::remove(this->stack_objects.begin(), this->stack_objects.end(), *to_delete);
    if(stack_iterator != this->stack_objects.end())
    {
        this->stack_objects.erase(stack_iterator);
    }
    auto heap_iterator = std::remove_if(this->heap_objects.begin(), this->heap_objects.end(), [&](const auto& object_ptr){return object_ptr.get() == to_delete;});
    if(heap_iterator != this->heap_objects.end())
    {
        this->heap_objects.erase(heap_iterator);
    }
}

void Scene::erase_sprite(Sprite* to_delete)
{
    auto stack_iterator = std::remove(this->stack_sprites.begin(), this->stack_sprites.end(), *to_delete);
    if(stack_iterator != this->stack_sprites.end())
    {
        this->stack_sprites.erase(stack_iterator);
    }
    auto heap_iterator = std::remove_if(this->heap_sprites.begin(), this->heap_sprites.end(), [&](const auto& sprite_ptr){return sprite_ptr.get() == to_delete;});
    if(heap_iterator != this->heap_sprites.end())
    {
        this->heap_sprites.erase(heap_iterator);
    }
}

void Scene::handle_deletions()
{
    for(StaticObject* deletion : this->objects_to_delete)
        this->erase_object(deletion);
    this->objects_to_delete.clear();
    for(Sprite* deletion : this->sprites_to_delete)
        this->erase_sprite(deletion);
    this->sprites_to_delete.clear();
}