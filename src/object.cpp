#include "object.hpp"

StaticObject::StaticObject(Transform transform, Asset asset): transform(transform), asset(asset){}

const Asset& StaticObject::get_asset() const
{
    return this->asset;
}

void StaticObject::render(Shader& render_shader, const Camera& camera, const Vector2<int>& viewport_dimensions) const
{
    if(!this->asset.valid_mesh())
    {
        std::cerr << "SceneObject attempted to be rendered with an invalid mesh. Aborted this draw-call.\n";
        return;
    }
    render_shader.bind();
    if(this->asset.valid_texture())
        this->asset.texture.lock()->bind(&render_shader, tz::graphics::texture_sampler_id);
    else
        tz::graphics::asset::unbind_texture();
    if(this->asset.valid_normal_map())
        this->asset.normal_map.lock()->bind(&render_shader, tz::graphics::normal_map_sampler_id);
    else
        tz::graphics::asset::unbind_normal_map(render_shader);
    if(this->asset.valid_parallax_map())
        this->asset.parallax_map.lock()->bind(&render_shader, tz::graphics::parallax_map_sampler_id);
    else
        tz::graphics::asset::unbind_parallax_map(render_shader);
    if(this->asset.valid_displacement_map())
        this->asset.displacement_map.lock()->bind(&render_shader, tz::graphics::displacement_map_sampler_id);
    else
        tz::graphics::asset::unbind_displacement_map(render_shader);
    render_shader.set_uniform<bool>("is_instanced", tz::graphics::is_instanced(this->asset.mesh.lock().get()));
    render_shader.set_uniform<Matrix4x4>(tz::graphics::render_shader_model_uniform_name, this->transform.model());
    render_shader.set_uniform<Matrix4x4>(tz::graphics::render_shader_view_uniform_name, camera.view());
    render_shader.set_uniform<Matrix4x4>(tz::graphics::render_shader_projection_uniform_name, camera.projection(viewport_dimensions.x, viewport_dimensions.y));
    render_shader.update();
    this->asset.mesh.lock()->render(render_shader.has_tessellation_control_shader());
}

InstancedStaticObject::InstancedStaticObject(const std::vector<StaticObject>& objects): StaticObject(Transform{{}, {}, {}}, {{}, {}, {}})
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
    this->instanced_mesh = std::make_shared<InstancedMesh>(objects.front().asset.mesh.lock()->get_file_name(), positions, rotations, scales);
    this->asset = objects.front().asset;
    this->asset.mesh = this->instanced_mesh;
}

void InstancedStaticObject::render(Shader& instanced_render_shader, const Camera& camera, const Vector2<int>& viewport_dimensions) const
{
    instanced_render_shader.bind();
    instanced_render_shader.set_uniform<bool>("is_instanced", true);
    instanced_render_shader.update();
    StaticObject::render(instanced_render_shader, camera, viewport_dimensions);
}

DynamicObject::DynamicObject(float mass, Transform transform, Asset asset, Vector3F velocity, Vector3F angular_velocity, std::initializer_list<Vector3F> forces): StaticObject(transform, asset), PhysicsObject(mass, velocity, angular_velocity, forces){}

void DynamicObject::update(float delta_time)
{
    PhysicsObject::update(delta_time);
    this->transform.position += (this->velocity * delta_time);
    this->transform.rotation += (this->angular_velocity * delta_time);
}