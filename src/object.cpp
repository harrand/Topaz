#include "object.hpp"

Object::Object(std::string mesh_link, std::map<Texture::TextureType, std::string> textures, Vector3F pos, Vector3F rot, Vector3F scale, unsigned int shininess, float parallax_map_scale, float parallax_map_offset, float displacement_factor): pos(std::move(pos)), rot(std::move(rot)), scale(std::move(scale)), shininess(shininess), parallax_map_scale(parallax_map_scale), parallax_map_offset(parallax_map_offset), displacement_factor(displacement_factor), mesh_link(std::move(mesh_link)), textures(std::move(textures)){}

const Vector3F& Object::get_position() const
{
	return this->pos;
}

const Vector3F& Object::get_rotation() const
{
	return this->rot;
}

const Vector3F& Object::get_scale() const
{
	return this->scale;
}

unsigned int Object::get_shininess() const
{
	return this->shininess;
}

float Object::get_parallax_map_scale() const
{
	return this->parallax_map_scale;
}

float Object::get_parallax_map_offset() const
{
	return this->parallax_map_offset;
}

float Object::get_displacement_factor() const
{
	return this->displacement_factor;
}

void Object::set_position(Vector3F position)
{
	this->pos = position;
}

void Object::set_rotation(Vector3F rotation)
{
	this->rot = rotation;
}

void Object::set_scale(Vector3F scale)
{
	this->scale = scale;
}

void Object::set_shininess(float shininess)
{
	this->shininess = shininess;
}

void Object::set_parallax_map_scale(float parallax_map_scale)
{
	this->parallax_map_scale = parallax_map_scale;
}

void Object::set_parallax_map_offset(float parallax_map_offset)
{
	this->parallax_map_offset = parallax_map_offset;
}

void Object::set_displacement_factor(float displacement_factor)
{
	this->displacement_factor = displacement_factor;
}

const std::string& Object::get_mesh_link() const
{
	return this->mesh_link;
}

const std::map<Texture::TextureType, std::string> Object::get_textures() const
{
	return this->textures;
}

void Object::render(Mesh* mesh, Texture* tex, NormalMap* nm, ParallaxMap* pm, DisplacementMap* dm, const Camera& cam, Shader& shad, float width, float height) const
{
	// O(1) CPU
	if(mesh == nullptr)
		return;
	shad.bind();
	if(tex != nullptr)
		tex->bind(shad.get_program_handle(), static_cast<unsigned int>(tex->get_texture_type()));
	if(nm != nullptr)
		nm->bind(shad.get_program_handle(), static_cast<unsigned int>(nm->get_texture_type()));
	if(pm != nullptr)
		pm->bind(shad.get_program_handle(), static_cast<unsigned int>(pm->get_texture_type()));
	if(dm != nullptr)
		dm->bind(shad.get_program_handle(), static_cast<unsigned int>(dm->get_texture_type()));
	shad.set_uniform<bool>("is_instanced", tz::graphics::is_instanced(mesh));
	shad.set_uniform<Matrix4x4>("m", Matrix4x4::create_model_matrix(this->pos, this->rot, this->scale));
	shad.set_uniform<Vector3F>("position_uniform", this->pos);
	shad.set_uniform<Vector3F>("rotation_uniform", this->rot);
	shad.set_uniform<Vector3F>("scale_uniform", this->scale);
	shad.set_uniform<Matrix4x4>("v", Matrix4x4::create_view_matrix(cam.get_position(), cam.get_rotation()));
	shad.set_uniform<Matrix4x4>("p", Matrix4x4::create_perspective_matrix(cam.get_fov(), width, height, cam.get_near_clip(), cam.get_far_clip()));
	shad.set_uniform<unsigned int>("shininess", this->shininess);
	shad.set_uniform<float>("parallax_map_scale", this->parallax_map_scale);
	shad.set_uniform<float>("parallax_map_offset", this->parallax_map_offset);
	shad.set_uniform<float>("displacement_factor", this->displacement_factor);
	shad.update();
	//shad.update(Matrix4x4::create_model_matrix(this->pos, this->rot, this->scale).fill_data(), Matrix4x4::create_view_matrix(cam.get_position(), cam.get_rotation()).fill_data(), Matrix4x4::create_perspective_matrix(cam.get_fov(), width, height, cam.get_near_clip(), cam.get_far_clip()).fill_data(), this->shininess, this->parallax_map_scale, this->parallax_map_offset, this->displacement_factor);
	//glFrontFace(GL_CCW);
	mesh->render(shad.has_tessellation_control_shader());
	//glFrontFace(GL_CW);
}

void Object::render(const std::vector<std::unique_ptr<Mesh>>& all_meshes, const std::vector<std::unique_ptr<Texture>>& all_textures, const std::vector<std::unique_ptr<NormalMap>>& all_normalmaps, const std::vector<std::unique_ptr<ParallaxMap>>& all_parallaxmaps, const std::vector<std::unique_ptr<DisplacementMap>>& all_displacementmaps, const Camera& cam, Shader& shad, float width, float height) const
{
	/*
	std::string texture_link, normalmap_link, parallaxmap_link, displacementmap_link;
	for(auto& texture : this->get_textures())
	{
		if(texture.first == Texture::TextureType::TEXTURE)
			texture_link = texture.second;
		if(texture.first == Texture::TextureType::NORMAL_MAP)
			normalmap_link = texture.second;
		if(texture.first == Texture::TextureType::PARALLAX_MAP)
			parallaxmap_link = texture.second;
		if(texture.first == Texture::TextureType::DISPLACEMENT_MAP)
			displacementmap_link = texture.second;
	}
	*/
	// std::map::operator[] is O(log n) so much better than iterating through map, but it does not have a const overload
	// however, std::map::at does have a const overload so that can be used in this function (still O(log n))
	// Texture::get_from_link and tz::graphics::find_mesh are O(n) but n is always going to be very small. As object counts will be far larger, they're considered O(1) amortised. This makes Object::render O(log n) amortised. Optimisation from O(n). This is only as far as the CPU is concerned of course, GPU processing will be massive for obvious reasons.
	std::string texture_link = this->textures.at(Texture::TextureType::TEXTURE);
	std::string normalmap_link = this->textures.at(Texture::TextureType::NORMAL_MAP);
	std::string parallaxmap_link = this->textures.at(Texture::TextureType::PARALLAX_MAP);
	std::string displacementmap_link = this->textures.at(Texture::TextureType::DISPLACEMENT_MAP);
	this->render(tz::graphics::find_mesh(this->get_mesh_link(), all_meshes), Texture::get_from_link<Texture>(texture_link, all_textures), Texture::get_from_link<NormalMap>(normalmap_link, all_normalmaps), Texture::get_from_link<ParallaxMap>(parallaxmap_link, all_parallaxmaps), Texture::get_from_link<DisplacementMap>(displacementmap_link, all_displacementmaps), cam, shad, width, height);
}

bool Object::operator==(const Object& rhs) const
{
	return this->pos == rhs.pos && this->rot == rhs.rot && this->scale == rhs.scale && this->shininess == rhs.shininess && this->parallax_map_scale == rhs.parallax_map_scale && this->parallax_map_offset == rhs.parallax_map_offset && this->displacement_factor == rhs.displacement_factor && this->mesh_link == rhs.mesh_link && this->textures == rhs.textures;
}

Skybox::Skybox(std::string cube_mesh_link, CubeMap& cm): cube_mesh_link(cube_mesh_link), cm(cm){}

void Skybox::render(const Camera& cam, Shader& shad, const std::vector<std::unique_ptr<Mesh>>& all_meshes, float width, float height)
{
	shad.bind();
	this->cm.bind(shad.get_program_handle(), 0);
	shad.set_uniform<Matrix4x4>("m", Matrix4x4::create_model_matrix(cam.get_position(), Vector3F(), Vector3F(cam.get_far_clip(), cam.get_far_clip(), cam.get_far_clip())));
	shad.set_uniform<Matrix4x4>("v", Matrix4x4::create_view_matrix(cam.get_position(), cam.get_rotation()));
	shad.set_uniform<Matrix4x4>("p", Matrix4x4::create_perspective_matrix(cam.get_fov(), width, height, cam.get_near_clip(), cam.get_far_clip()));
	shad.set_uniform<unsigned int>("shininess", 0);
	shad.set_uniform<float>("parallax_map_scale", 0);
	shad.set_uniform<float>("parallax_map_offset", 0);
	shad.set_uniform<float>("displacement_factor", 0);
	shad.update();
	//shad.update(Matrix4x4::create_model_matrix(cam.get_position(), Vector3F(), Vector3F(cam.get_far_clip(), cam.get_far_clip(), cam.get_far_clip())).fill_data(), Matrix4x4::create_view_matrix(cam.get_position(), cam.get_rotation()).fill_data(), Matrix4x4::create_perspective_matrix(cam.get_fov(), width, height, cam.get_near_clip(), cam.get_far_clip()).fill_data(), 0, 0, 0, 0);
	glFrontFace(GL_CW);
	tz::graphics::find_mesh(this->cube_mesh_link, all_meshes)->render(shad.has_tessellation_control_shader());
	glFrontFace(GL_CCW);
}