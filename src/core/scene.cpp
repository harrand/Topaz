//
// Created by Harrand on 29/05/2018.
//

#include <physics/physics.hpp>
#include "scene.hpp"

Scene::Scene(const std::initializer_list<StaticObject>& stack_objects, const std::initializer_list<std::shared_ptr<StaticObject>>& heap_objects): stack_objects(stack_objects), heap_objects(heap_objects){}

void Scene::render(Shader& render_shader, const Camera& camera, const Vector2I& viewport_dimensions) const
{
    Frustum camera_frustum(camera, viewport_dimensions.x / viewport_dimensions.y);
    auto render_if_visible = [&](const StaticObject& object){AABB object_box = tz::physics::bound_aabb(*(object.get_asset().mesh.lock())); if(camera_frustum.contains(object_box * object.transform.model()) || tz::graphics::is_instanced(object.get_asset().mesh.lock().get())) object.render(render_shader, camera, viewport_dimensions);};
    for(const StaticObject& stack_object : this->stack_objects)
        render_if_visible(stack_object);
    for(std::shared_ptr<StaticObject> heap_object : this->heap_objects)
        render_if_visible(*heap_object);
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
    for(const std::shared_ptr<StaticObject>& object_ptr : this->heap_objects)
        object_crefs.push_back(std::cref(*object_ptr));
    return object_crefs;
}

void Scene::add_object(StaticObject scene_object)
{
    this->stack_objects.push_back(scene_object);
}

std::vector<std::reference_wrapper<StaticObject>> Scene::get_static_objects()
{
    std::vector<std::reference_wrapper<StaticObject>> object_refs;
    for(StaticObject& object_ref : this->stack_objects)
        object_refs.push_back(std::ref(object_ref));
    for(std::shared_ptr<StaticObject>& object_ptr : this->heap_objects)
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