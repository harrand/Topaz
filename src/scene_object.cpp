#include "scene_object.hpp"

SceneObject::SceneObject(Transform transform, Asset asset): transform(transform), asset(asset){}

const Asset& SceneObject::get_asset() const
{
    return this->asset;
}

void SceneObject::render(Shader& render_shader, const Camera& camera, const Vector2<int>& viewport_dimensions) const
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
    render_shader.set_uniform<bool>("is_instanced", false);
    render_shader.set_uniform<Matrix4x4>(tz::graphics::render_shader_model_uniform_name, this->transform.model());
    render_shader.set_uniform<Matrix4x4>(tz::graphics::render_shader_view_uniform_name, camera.view());
    render_shader.set_uniform<Matrix4x4>(tz::graphics::render_shader_projection_uniform_name, camera.projection(viewport_dimensions.x, viewport_dimensions.y));
    render_shader.update();
    this->asset.mesh.lock()->render(render_shader.has_tessellation_control_shader());
}
