#include "core/scene.hpp"
#include "physics/physics.hpp"

Scene::Scene(const std::initializer_list<StaticObject>& stack_objects, std::vector<std::unique_ptr<StaticObject>> heap_objects): stack_objects(stack_objects), heap_objects(std::move(heap_objects)), directional_lights{}/*, point_lights{}*/{}

void Scene::render(Shader& render_shader, const Camera& camera, const Vector2I& viewport_dimensions) const
{
    Frustum camera_frustum(camera, viewport_dimensions.x / viewport_dimensions.y);
    auto render_if_visible = [&](const StaticObject& object){AABB object_box = tz::physics::bound_aabb(*(object.get_asset().mesh)); if(camera_frustum.contains(object_box * object.transform.model()) || tz::graphics::is_instanced(object.get_asset().mesh)) object.render(render_shader, camera, viewport_dimensions);};
    for(const StaticObject& stack_object : this->stack_objects)
        render_if_visible(stack_object);
    for(const auto& heap_object : this->heap_objects)
        render_if_visible(*heap_object);
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
    for(std::reference_wrapper<DynamicObject> dynamic_ref : this->get_dynamic_objects())
        dynamic_ref.get().update(delta_time);
    std::vector<std::reference_wrapper<PhysicsObject>> physics_objects;
    for(auto& object : this->get_static_objects())
    {
        auto physics_component = dynamic_cast<PhysicsObject*>(&object.get());
        if(physics_component != nullptr)
            physics_objects.push_back(*physics_component);
    }
    for(auto& physics_ref : physics_objects)
        physics_ref.get().handle_collisions(physics_objects);
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

AABB Scene::get_boundary() const
{
    auto objects = this->get_objects();
    if(objects.size() == 0)
        return {{}, {}};
    Vector3F min = objects.front().get().transform.position, max = objects.front().get().transform.position;
    for(const StaticObject& object : objects)
    {
        min.x = std::min(min.x, object.transform.position.x);
        min.y = std::min(min.y, object.transform.position.y);
        min.z = std::min(min.z, object.transform.position.z);

        max.x = std::max(max.x, object.transform.position.x);
        max.y = std::max(max.y, object.transform.position.y);
        max.z = std::max(max.z, object.transform.position.z);
    }
    return {min, max};
}

void Scene::add_object(StaticObject scene_object)
{
    this->stack_objects.push_back(scene_object);
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

std::vector<std::reference_wrapper<StaticObject>> Scene::get_static_objects()
{
    std::vector<std::reference_wrapper<StaticObject>> object_refs;
    for(StaticObject& object_ref : this->stack_objects)
        object_refs.push_back(std::ref(object_ref));
    for(std::unique_ptr<StaticObject>& object_ptr : this->heap_objects)
        object_refs.push_back(std::ref(*object_ptr));
    return object_refs;
}

std::vector<std::reference_wrapper<DynamicObject>> Scene::get_dynamic_objects()
{
    std::vector<std::reference_wrapper<DynamicObject>> object_refs;
    for(std::reference_wrapper<StaticObject> static_ref : this->get_static_objects())
    {
        DynamicObject* dynamic_ref = dynamic_cast<DynamicObject*>(&static_ref.get());
        if(dynamic_ref != nullptr)
            object_refs.push_back(std::ref(*dynamic_ref));
    }
    return object_refs;
}