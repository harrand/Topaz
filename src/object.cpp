#include "object.hpp"

Object2D::Object2D(Vector2F position, float rotation, Vector2F scale, Vector4F colour): position(position), scale(scale), rotation(rotation), colour(colour), quad(tz::graphics::create_quad()){}

void Object2D::render(const Camera& cam, Shader* shader, float width, float height) const
{
	shader->bind();
	shader->set_uniform<Matrix4x4>("m", tz::transform::model(Vector3F(this->position, 0.0f), Vector3F(0.0f, 0.0f, this->rotation), Vector3F(this->scale, 1.0f)));
	shader->set_uniform<Matrix4x4>("v", tz::transform::view(cam.position, cam.rotation));
	shader->set_uniform<Matrix4x4>("p", cam.projection(width, height));
	shader->set_uniform<Vector4F>("colour", this->colour);
	shader->set_uniform<bool>("has_texture", false);
	shader->update();
	this->quad.render(false);
}

Object3D::Object3D(std::variant<const Mesh*, std::shared_ptr<const Mesh>> mesh, std::map<tz::graphics::TextureType, Texture*> textures, Vector3F position, Vector3F rotation, Vector3F scale, unsigned int shininess, float parallax_map_scale, float parallax_map_offset, float displacement_factor): position(position), rotation(rotation), scale(scale), shininess(shininess), parallax_map_scale(parallax_map_scale), parallax_map_offset(parallax_map_offset), displacement_factor(displacement_factor), mesh(mesh), textures(textures){}

const Mesh& Object3D::get_mesh() const
{
	try
	{
		return *std::get<0>(this->mesh);
	}catch(std::bad_variant_access&)
	{
		return *std::get<1>(this->mesh);
	}
}

const std::map<tz::graphics::TextureType, Texture*>& Object3D::get_textures() const
{
	return this->textures;
}


void Object3D::render(const Camera& cam, Shader* shader, float width, float height) const
{
	if(&(this->get_mesh()) == nullptr)
	{
		tz::util::log::error("Attempted to render Object3D with a null mesh. Aborting render process.");
		return;
	}
	using tz::graphics::TextureType;
	shader->bind();
	// Don't need the bounds-checking from std::map::at() but using it because std::map::operator[] has no const overload (which is dumb, but hey. The const correctness should provide a bigger optimisation than the overhead incurred by using std::map::at)
	Texture* texture = this->textures.at(TextureType::TEXTURE);
	NormalMap* normal_map = dynamic_cast<NormalMap*>(this->textures.at(TextureType::NORMAL_MAP));
	ParallaxMap* parallax_map = dynamic_cast<ParallaxMap*>(this->textures.at(TextureType::PARALLAX_MAP));
	DisplacementMap* displacement_map = dynamic_cast<DisplacementMap*>(this->textures.at(TextureType::DISPLACEMENT_MAP));
	if(texture != nullptr)
		texture->bind(shader, static_cast<unsigned int>(texture->get_texture_type()));
	if(normal_map != nullptr)
		normal_map->bind(shader, static_cast<unsigned int>(normal_map->get_texture_type()));
	if(parallax_map != nullptr)
		parallax_map->bind(shader, static_cast<unsigned int>(parallax_map->get_texture_type()));
	if(displacement_map != nullptr)
		displacement_map->bind(shader, static_cast<unsigned int>(displacement_map->get_texture_type()));
	shader->set_uniform<bool>("is_instanced", tz::graphics::is_instanced(&(this->get_mesh())));
	shader->set_uniform<Matrix4x4>("m", tz::transform::model(this->position, this->rotation, this->scale));
	shader->set_uniform<Vector3F>("position_uniform", this->position);
	shader->set_uniform<Vector3F>("rotation_uniform", this->rotation);
	shader->set_uniform<Vector3F>("scale_uniform", this->scale);
	shader->set_uniform<Matrix4x4>("v", tz::transform::view(cam.position, cam.rotation));
	shader->set_uniform<Matrix4x4>("p", cam.projection(width, height));
	shader->set_uniform<unsigned int>("shininess", this->shininess);
	shader->set_uniform<float>("parallax_map_scale", this->parallax_map_scale);
	shader->set_uniform<float>("parallax_map_offset", this->parallax_map_offset);
	shader->set_uniform<float>("displacement_factor", this->displacement_factor);
	shader->update();
	this->get_mesh().render(shader->has_tessellation_control_shader());
	
}

bool Object3D::operator==(const Object3D& rhs) const
{
	return this->position == rhs.position && this->rotation == rhs.rotation && this->scale == rhs.scale && this->shininess == rhs.shininess && this->parallax_map_scale == rhs.parallax_map_scale && this->parallax_map_offset == rhs.parallax_map_offset && this->displacement_factor == rhs.displacement_factor && this->mesh == rhs.mesh && this->textures == rhs.textures;
}

Skybox::Skybox(std::string cube_mesh_link, CubeMap& cm): cube_mesh_link(cube_mesh_link), cm(cm){}

void Skybox::render(const Camera& cam, Shader& shad, const std::vector<std::unique_ptr<Mesh>>& all_meshes, float width, float height)
{
	shad.bind();
	this->cm.bind(&shad, 0);
	shad.set_uniform<Matrix4x4>("m", tz::transform::model(cam.position, Vector3F(), Vector3F(cam.far_clip, cam.far_clip, cam.far_clip)));
	shad.set_uniform<Matrix4x4>("v", tz::transform::view(cam.position, cam.rotation));
	shad.set_uniform<Matrix4x4>("p", tz::transform::perspective_projection(cam.fov, width, height, cam.near_clip, cam.far_clip));
	shad.set_uniform<unsigned int>("shininess", 0);
	shad.set_uniform<float>("parallax_map_scale", 0);
	shad.set_uniform<float>("parallax_map_offset", 0);
	shad.set_uniform<float>("displacement_factor", 0);
	shad.update();
	glFrontFace(GL_CW);
	tz::graphics::find_mesh(this->cube_mesh_link, all_meshes)->render(shad.has_tessellation_control_shader());
	glFrontFace(GL_CCW);
}