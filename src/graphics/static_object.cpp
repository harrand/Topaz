#include "graphics/static_object.hpp"

StaticObject::StaticObject(Transform transform, Asset asset): transform(transform), asset(asset){}

const Asset& StaticObject::get_asset() const
{
	return this->asset;
}

std::optional<AABB> StaticObject::get_boundary() const
{
	return tz::physics::bound_object(*this);
}

void StaticObject::render(RenderPass render_pass) const
{
	if(!this->asset.valid_mesh() && !this->asset.valid_model())
	{
		tz::debug::print("StaticObject::render(...): Error: StaticObject attempted to be rendered with neither valid mesh nor model. Aborted this draw-call.\n");
		return;
	}
	Shader& render_shader = *render_pass.get_render_context().object_shader;
	const Camera& camera = render_pass.get_camera();
	Vector2I viewport_dimensions = {render_pass.get_window().get_width(), render_pass.get_window().get_height()};
	render_shader.bind();
	render_shader.set_uniform<bool>("is_instanced", this->get_asset().valid_mesh() && tz::graphics::is_instanced(this->asset.mesh));
	render_shader.set_uniform<Matrix4x4>(tz::graphics::render_shader_model_uniform_name, this->transform.model());
	render_shader.set_uniform<Matrix4x4>(tz::graphics::render_shader_view_uniform_name, camera.view());
	render_shader.set_uniform<Matrix4x4>(tz::graphics::render_shader_projection_uniform_name,
										 camera.projection(viewport_dimensions.x, viewport_dimensions.y));
	tz::graphics::asset::unbind_all_extra_textures(render_shader);
	tz::graphics::asset::unbind_specular_map(render_shader);
	tz::graphics::asset::unbind_emissive_map(render_shader);
	if (this->asset.valid_texture())
		this->asset.texture->bind(&render_shader, tz::graphics::texture_sampler_id);
	else
		tz::graphics::asset::unbind_texture();
	if (this->asset.valid_normal_map())
		this->asset.normal_map->bind(&render_shader, tz::graphics::normal_map_sampler_id);
	else
		tz::graphics::asset::unbind_normal_map(render_shader);
	if (this->asset.valid_parallax_map())
		this->asset.parallax_map->bind(&render_shader, tz::graphics::parallax_map_sampler_id);
	else
		tz::graphics::asset::unbind_parallax_map(render_shader);
	if (this->asset.valid_displacement_map())
		this->asset.displacement_map->bind(&render_shader, tz::graphics::displacement_map_sampler_id);
	else
		tz::graphics::asset::unbind_displacement_map(render_shader);
	if(this->asset.valid_specular_map())
		this->asset.specular_map->bind(&render_shader, tz::graphics::specular_map_sampler_id);
	else
		tz::graphics::asset::unbind_specular_map(render_shader);
	if(this->asset.valid_emissive_map())
		this->asset.emissive_map->bind(&render_shader, tz::graphics::emissive_map_sampler_id);
	else
		tz::graphics::asset::unbind_emissive_map(render_shader);
	render_shader.update();
	if(!this->asset.valid_model())
	{
		this->asset.mesh->render(render_shader.get_can_tessellate());
	}
	else
	{
		Model* model = this->asset.model;
		model->render(render_shader);
	}
}

bool  StaticObject::operator==(const StaticObject &rhs) const
{
	return this->transform == rhs.transform && this->asset == rhs.asset;
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
	this->instanced_mesh = std::make_shared<InstancedMesh>(*objects.front().asset.mesh, positions, rotations, scales);
	this->asset = objects.front().asset;
	this->asset.mesh = this->instanced_mesh.get();
}

const InstancedMesh& InstancedStaticObject::get_instanced_mesh() const
{
	return *this->instanced_mesh;
}

std::optional<AABB> InstancedStaticObject::get_boundary() const
{
	std::optional<AABB> bound = StaticObject::get_boundary();
	if(!bound.has_value())
		return std::nullopt;
	for(const Matrix4x4& model_matrix : this->instanced_mesh->get_model_matrices())
	{
		std::optional<AABB> boundary = tz::physics::bound_aabb(*this->instanced_mesh, model_matrix);
		if(boundary.has_value())
			bound.value() = bound.value().expand_to(boundary.value());
	}
	return bound;
}

void InstancedStaticObject::render(RenderPass render_pass) const
{
	Shader& instanced_render_shader = *render_pass.get_render_context().object_shader;
	instanced_render_shader.bind();
	instanced_render_shader.set_uniform<bool>("is_instanced", true);
	instanced_render_shader.update();
	StaticObject::render(render_pass);
}