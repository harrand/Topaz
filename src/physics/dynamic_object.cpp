#include "physics/dynamic_object.hpp"
#include "physics/physics.hpp"

DynamicObject::DynamicObject(float mass, Transform transform, Asset asset, Vector3F velocity, float moment_of_inertia, Vector3F angular_velocity, std::initializer_list<Vector3F> forces): StaticObject(transform, asset), PhysicsObject(mass, velocity, moment_of_inertia, angular_velocity, forces), bound_modelspace({}, {})
{
    if(this->asset.valid_mesh())
        this->bound_modelspace = tz::physics::bound_aabb(*this->asset.mesh);
}

void DynamicObject::update(float delta_time)
{
    PhysicsObject::update(delta_time);
    this->transform.position += (this->velocity * delta_time);
    this->transform.rotation += (this->angular_velocity * delta_time);
}

AABB DynamicObject::get_boundary() const
{
    return this->bound_modelspace * this->transform.model();
}

void DynamicObject::on_collision(PhysicsObject &collided)
{

}

InstancedDynamicObject::InstancedDynamicObject(const std::vector<DynamicObject>& objects): DynamicObject(0.0f, Transform{{}, {}, {}}, Asset{{}, {}}), instanced_mesh(nullptr), objects(objects)
{
    std::vector<Vector3F> positions, rotations, scales;
    Vector3F original_position = objects.front().transform.position;
    Vector3F original_rotation = objects.front().transform.rotation;
    Vector3F original_scale = objects.front().transform.scale;
    for(const StaticObject& object : objects)
    {
        positions.push_back(object.transform.position);
        rotations.push_back(object.transform.rotation);
        scales.push_back(object.transform.scale);
    }
    this->transform = {original_position, original_rotation, original_scale};
    this->instanced_mesh = std::make_shared<InstancedMesh>(objects.front().asset.mesh->get_file_name(), positions, rotations, scales, true);
    this->asset = objects.front().asset;
    this->asset.mesh = this->instanced_mesh.get();
}

void InstancedDynamicObject::update(float delta_time)
{
    for(std::size_t instance_id = 0; instance_id < this->objects.size(); instance_id++)
    {
        auto& dynamic_object = this->objects[instance_id];
        dynamic_object.update(delta_time);
        this->instanced_mesh->set_instance_position(instance_id, dynamic_object.transform.position);
        this->instanced_mesh->set_instance_rotation(instance_id, dynamic_object.transform.rotation);
        this->instanced_mesh->set_instance_scale(instance_id, dynamic_object.transform.scale);
    }
}

void InstancedDynamicObject::render(Shader& instanced_render_shader, const Camera& camera, const Vector2I& viewport_dimensions) const
{
    instanced_render_shader.bind();
    instanced_render_shader.set_uniform<bool>("is_instanced", true);
    instanced_render_shader.update();
    DynamicObject::render(instanced_render_shader, camera, viewport_dimensions);
}