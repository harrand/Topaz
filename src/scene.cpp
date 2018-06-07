//
// Created by Harrand on 29/05/2018.
//

#include <physics/physics.hpp>
#include "scene.hpp"

Scene::Scene(const std::initializer_list<SceneObject> stack_objects, const std::initializer_list<std::shared_ptr<SceneObject>> heap_objects): stack_objects(stack_objects), heap_objects(heap_objects){}

void Scene::render(Shader& render_shader, const Camera& camera, const Vector2<int>& viewport_dimensions) const
{
    /*
    for(auto object_cref : this->get_objects())
    {
        object_cref.get().render(render_shader, camera, viewport_dimensions);
    }
     */
    Frustum camera_frustum(camera, viewport_dimensions.x / viewport_dimensions.y);
    for(auto object_cref : this->get_objects())
    {
        AABB object_box = tz::physics::bound_aabb(*(object_cref.get().get_asset().mesh.lock()));
        if(camera_frustum.contains(object_box * object_cref.get().transform.model()))
            object_cref.get().render(render_shader, camera, viewport_dimensions);
    }
}

std::vector<std::reference_wrapper<const SceneObject>> Scene::get_objects() const
{
    std::vector<std::reference_wrapper<const SceneObject>> object_crefs;
    for(const SceneObject& object_cref : this->stack_objects)
        object_crefs.push_back(std::cref(object_cref));
    for(const std::shared_ptr<SceneObject>& object_ptr : this->heap_objects)
        object_crefs.push_back(std::cref(*object_ptr));
    return object_crefs;
}

void Scene::add_object(SceneObject scene_object)
{
    this->stack_objects.push_back(scene_object);
}