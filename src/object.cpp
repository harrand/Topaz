#include "object.hpp"

Object::Object(const Mesh* mesh, std::map<tz::graphics::TextureType, Texture*> textures, Vector3F position, Vector3F rotation, Vector3F scale, unsigned int shininess, float parallax_map_scale, float parallax_map_offset, float displacement_factor): position(position), rotation(rotation), scale(scale), shininess(shininess), parallax_map_scale(parallax_map_scale), parallax_map_offset(parallax_map_offset), displacement_factor(displacement_factor), mesh(mesh), textures(textures){}

const Mesh& Object::get_mesh() const
{
	return *(this->mesh);
}

const std::map<tz::graphics::TextureType, Texture*>& Object::get_textures() const
{
	return this->textures;
}


void Object::render(const Camera& cam, Shader* shader, float width, float height)
{
	if(this->mesh == nullptr)
	{
		tz::util::log::error("Attempted to render Object with a null mesh. Aborting render process.");
		return;
	}
	using tz::graphics::TextureType;
	shader->bind();
	Texture* texture = this->textures[TextureType::TEXTURE];
	NormalMap* normal_map = dynamic_cast<NormalMap*>(this->textures[TextureType::NORMAL_MAP]);
	ParallaxMap* parallax_map = dynamic_cast<ParallaxMap*>(this->textures[TextureType::PARALLAX_MAP]);
	DisplacementMap* displacement_map = dynamic_cast<DisplacementMap*>(this->textures[TextureType::DISPLACEMENT_MAP]);
	if(texture != nullptr)
		texture->bind(shader, static_cast<unsigned int>(texture->get_texture_type()));
	if(normal_map != nullptr)
		normal_map->bind(shader, static_cast<unsigned int>(normal_map->get_texture_type()));
	if(parallax_map != nullptr)
		parallax_map->bind(shader, static_cast<unsigned int>(parallax_map->get_texture_type()));
	if(displacement_map != nullptr)
		displacement_map->bind(shader, static_cast<unsigned int>(displacement_map->get_texture_type()));
	shader->set_uniform<bool>("is_instanced", tz::graphics::is_instanced(mesh));
	shader->set_uniform<Matrix4x4>("m", Matrix4x4::create_model_matrix(this->position, this->rotation, this->scale));
	shader->set_uniform<Vector3F>("position_uniform", this->position);
	shader->set_uniform<Vector3F>("rotation_uniform", this->rotation);
	shader->set_uniform<Vector3F>("scale_uniform", this->scale);
	shader->set_uniform<Matrix4x4>("v", Matrix4x4::create_view_matrix(cam.position, cam.rotation));
	shader->set_uniform<Matrix4x4>("p", Matrix4x4::create_perspective_matrix(cam.fov, width, height, cam.near_clip, cam.far_clip));
	shader->set_uniform<unsigned int>("shininess", this->shininess);
	shader->set_uniform<float>("parallax_map_scale", this->parallax_map_scale);
	shader->set_uniform<float>("parallax_map_offset", this->parallax_map_offset);
	shader->set_uniform<float>("displacement_factor", this->displacement_factor);
	shader->update();
	mesh->render(shader->has_tessellation_control_shader());
	
}

bool Object::operator==(const Object& rhs) const
{
	return this->position == rhs.position && this->rotation == rhs.rotation && this->scale == rhs.scale && this->shininess == rhs.shininess && this->parallax_map_scale == rhs.parallax_map_scale && this->parallax_map_offset == rhs.parallax_map_offset && this->displacement_factor == rhs.displacement_factor && this->mesh == rhs.mesh && this->textures == rhs.textures;
}

Skybox::Skybox(std::string cube_mesh_link, CubeMap& cm): cube_mesh_link(cube_mesh_link), cm(cm){}

void Skybox::render(const Camera& cam, Shader& shad, const std::vector<std::unique_ptr<Mesh>>& all_meshes, float width, float height)
{
	shad.bind();
	this->cm.bind(&shad, 0);
	shad.set_uniform<Matrix4x4>("m", Matrix4x4::create_model_matrix(cam.position, Vector3F(), Vector3F(cam.far_clip, cam.far_clip, cam.far_clip)));
	shad.set_uniform<Matrix4x4>("v", Matrix4x4::create_view_matrix(cam.position, cam.rotation));
	shad.set_uniform<Matrix4x4>("p", Matrix4x4::create_perspective_matrix(cam.fov, width, height, cam.near_clip, cam.far_clip));
	shad.set_uniform<unsigned int>("shininess", 0);
	shad.set_uniform<float>("parallax_map_scale", 0);
	shad.set_uniform<float>("parallax_map_offset", 0);
	shad.set_uniform<float>("displacement_factor", 0);
	shad.update();
	glFrontFace(GL_CW);
	tz::graphics::find_mesh(this->cube_mesh_link, all_meshes)->render(shad.has_tessellation_control_shader());
	glFrontFace(GL_CCW);
}

Object tz::graphics::instancify(const std::vector<Object>& objects)
{
	std::vector<Vector3F> positions, rotations, scales;
	Vector3F original_position = objects.front().position;
	Vector3F original_rotation = objects.front().rotation;
	Vector3F original_scale = objects.front().scale;
	for(const Object& object : objects)
	{
		positions.push_back(object.position - original_position);
		rotations.push_back(object.rotation - original_rotation);
		scales.push_back(object.scale - original_scale);
	}
	// this will leak.
	InstancedMesh* mesh = new InstancedMesh(objects.front().get_mesh().get_file_name(), positions, rotations, scales);
	tz::util::log::message("instancified result size = ", mesh->get_instance_quantity());
	tz::util::log::message("detected instanced mesh: ", tz::graphics::is_instanced(mesh));
	return {mesh, objects.front().get_textures(), original_position, original_rotation, original_scale};
}