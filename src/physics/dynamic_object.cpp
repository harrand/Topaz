#include "physics/dynamic_object.hpp"

DynamicObject::DynamicObject(float mass, Transform transform, Asset asset, Vector3F velocity, float moment_of_inertia, Vector3F angular_velocity, std::initializer_list<Vector3F> forces): StaticObject(transform, asset), PhysicsObject(mass, velocity, moment_of_inertia, angular_velocity, forces), bound_modelspace({}, {})
{
    if(this->asset.valid_mesh())
        this->bound_modelspace = tz::physics::bound_aabb(*this->asset.mesh);
}

void DynamicObject::update(float delta_time)
{
    // This lambda performs verlet-integration. This is cheaper than forest-ruth but is slightly less accurate.
    auto verlet_integration = [&](float delta)
            {
                float half_delta = delta * 0.5f;
                this->transform.position += (this->velocity * half_delta);
                this->transform.rotation += (this->angular_velocity * half_delta);
                PhysicsObject::update(delta_time);
                this->transform.position += (this->velocity * half_delta);
                this->transform.rotation += (this->angular_velocity * half_delta);
            };
    // Perform forest-ruth motion integration. It utilises verlet-integration, so it more expensive but yields more accurate results.
    using namespace tz::utility::numeric;
    verlet_integration(delta_time * static_cast<float>(consts::forest_ruth_coefficient));
    verlet_integration(delta_time * static_cast<float>(consts::forest_ruth_complement));
    verlet_integration(delta_time * static_cast<float>(consts::forest_ruth_coefficient));
}

std::optional<AABB> DynamicObject::get_boundary() const
{
    return StaticObject::get_boundary();
}

void DynamicObject::on_collision([[maybe_unused]] PhysicsObject &collided) {}

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
    this->instanced_mesh = std::make_shared<InstancedMesh>(*objects.front().asset.mesh, positions, rotations, scales, true);
    this->asset = objects.front().asset;
    this->asset.mesh = this->instanced_mesh.get();
}

float InstancedDynamicObject::get_mass() const
{
    float mass = 0.0f;
    for(const DynamicObject& object : this->objects)
        mass += object.mass;
    return mass;
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

std::optional<AABB> InstancedDynamicObject::get_boundary() const
{
    using OptAABB = std::optional<AABB>;
    OptAABB this_bound = DynamicObject::get_boundary();
    if(!this_bound.has_value())
        return std::nullopt;
    for(const DynamicObject& object : this->objects)
    {
        OptAABB bound = object.get_boundary();
        if(!bound.has_value())
            continue;
        AABB just_bound = bound.value();
        this_bound = this_bound->expand_to(just_bound);
    }
    return this_bound;
}

void InstancedDynamicObject::on_collision([[maybe_unused]] PhysicsObject &collided){}