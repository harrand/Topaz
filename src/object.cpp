#include "object.hpp"

Object::Object(std::variant<const Mesh*, std::shared_ptr<const Mesh>> mesh, std::map<tz::graphics::TextureType, Texture*> textures, Vector3F position, Vector3F rotation, Vector3F scale, unsigned int shininess, float parallax_map_scale, float parallax_map_offset, float displacement_factor): position(position), rotation(rotation), scale(scale), shininess(shininess), parallax_map_scale(parallax_map_scale), parallax_map_offset(parallax_map_offset), displacement_factor(displacement_factor), mesh(mesh), textures(textures){}

const Mesh& Object::get_mesh() const
{
	try
	{
		return *std::get<0>(this->mesh);
	}catch(std::bad_variant_access&)
	{
		return *std::get<1>(this->mesh);
	}
}

const std::map<tz::graphics::TextureType, Texture*>& Object::get_textures() const
{
	return this->textures;
}


void Object::render(const Camera& cam, Shader* shader, float width, float height)
{
	if(&(this->get_mesh()) == nullptr)
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
	shader->set_uniform<bool>("is_instanced", tz::graphics::is_instanced(&(this->get_mesh())));
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
	this->get_mesh().render(shader->has_tessellation_control_shader());
	
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
	return {std::make_shared<InstancedMesh>(objects.front().get_mesh().get_file_name(), positions, rotations, scales), objects.front().get_textures(), original_position, original_rotation, original_scale};
}

std::vector<Object> tz::graphics::instancify_full(const std::vector<Object>& objects)
{
	using mesh_cref_t = std::reference_wrapper<const Mesh>;
	using textures_cref_t = std::reference_wrapper<const std::map<tz::graphics::TextureType, Texture*>>;
	auto useless_comparator = []([[maybe_unused]] auto lhs, [[maybe_unused]] auto rhs){return true;};
	std::multimap<std::pair<mesh_cref_t, textures_cref_t>, std::size_t, std::function<bool(std::pair<mesh_cref_t, textures_cref_t>, std::pair<mesh_cref_t, textures_cref_t>)>> asset_mappings(useless_comparator);
	for(std::size_t i = 0; i < objects.size(); i++)
	{
		const Object& object = objects[i];
		mesh_cref_t mesh_read = std::cref(object.get_mesh());
		textures_cref_t textures_read = std::cref(object.get_textures());
		asset_mappings.emplace(std::make_pair(mesh_read, textures_read), i);
		tz::util::log::message("emplaced asset pair to multimap.");
	}
	std::vector<Object> instancified_objects;
	std::vector<Object> duplicates;
	std::pair<mesh_cref_t, textures_cref_t> key_cache = asset_mappings.begin()->first;
	bool last_ends_cluster = false;
	for(const auto& pair : asset_mappings)
	{
		if(pair.first.first.get() == key_cache.first.get() && pair.first.second.get() == key_cache.second.get())
		{
			duplicates.push_back(objects[pair.second]);
			tz::util::log::message("found an element of a cluster.");
			last_ends_cluster = false;
		}
		else
		{
			key_cache = pair.first;
			instancified_objects.push_back(tz::graphics::instancify(duplicates));
			tz::util::log::message("finished cluster, instancifying...");
			duplicates.clear();
			last_ends_cluster = true;
			// we still need to add the current object to a new duplicates vector though or we'll skip it.
			duplicates.push_back(objects[pair.second]);
		}
	}
	if(!last_ends_cluster)
	{
		instancified_objects.push_back(tz::graphics::instancify(duplicates));
		tz::util::log::message("instancified last cluster!");
	}
	tz::util::log::message("number of instancified objects = ", instancified_objects.size());
	tz::util::log::message("number of previously uninstancified objects = ", objects.size());
	tz::util::log::message("instancification ratio (%) = ", 100.0f * instancified_objects.size() / objects.size(), "%. this also serves as the amount of processing performed as opposed to without instancification.");
	std::size_t total_instance_count = 0;
	for(const Object& object : instancified_objects)
		total_instance_count += dynamic_cast<const InstancedMesh*>(&(object.get_mesh()))->get_instance_quantity();
	tz::util::log::message("original number of objects = ", objects.size(), ", total instances created = ", total_instance_count);
	return instancified_objects;
}