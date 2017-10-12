#include "world.hpp"
#include "data.hpp"

World::World(std::string filename, std::string resources_path): filename(std::move(filename)), resources_path(std::move(resources_path))
{
	MDLF input(RawFile(this->filename));
	std::string spawn_point_string = input.get_tag("spawnpoint"), spawn_orientation_string = input.get_tag("spawnorientation"), gravity_string = input.get_tag("gravity");
	if(spawn_point_string != mdl::default_string && spawn_orientation_string != mdl::default_string && gravity_string != mdl::default_string)
	{
		this->spawn_point = tz::util::string::vectorise_list_3<float>(tz::util::string::deformat(spawn_point_string));
		this->spawn_orientation = tz::util::string::vectorise_list_3<float>(tz::util::string::deformat(spawn_orientation_string));
		this->gravity = tz::util::string::vectorise_list_3<float>(tz::util::string::deformat(gravity_string));
	}
	else
	{
		this->spawn_point = Vector3F();
		this->spawn_orientation = Vector3F();
		this->gravity = Vector3F();
	}
	std::vector<std::string> object_list = input.get_sequence("objects");
	std::vector<std::string> entity_object_list = input.get_sequence("entityobjects");
	for(std::string object_name : object_list)
		this->add_object(World::retrieve_object_data(object_name, this->resources_path, input));
	for(std::string entity_object_name : entity_object_list)
		this->add_entity_object(World::retrieve_entity_object_data(entity_object_name, this->resources_path, input));
}

World::World(const World& copy): World(copy.filename){}

World::World(World&& move): filename(move.filename), resources_path(move.resources_path), gravity(move.gravity), spawn_point(move.spawn_point), spawn_orientation(move.spawn_orientation), objects(move.objects), entities(move.entities), entity_objects(std::move(move.entity_objects)), base_lights(std::move(move.base_lights)){}

World::~World()
{
	this->kill_lights();
}

const std::string& World::get_file_name() const
{
	return this->filename;
}

const Vector3F& World::get_gravity() const
{
	return this->gravity;
}

const Vector3F& World::get_spawn_point() const
{
	return this->spawn_point;
}

const Vector3F& World::get_spawn_orientation() const
{
	return this->spawn_orientation;
}

void World::set_gravity(Vector3F gravity)
{
	this->gravity = gravity;
	for(Entity& ent : this->entities)
	{
		ent.remove_force("gravity");
		ent.apply_force("gravity", Force(this->get_gravity()));
	}
	for(EntityObject& eo : this->entity_objects)
	{
		eo.remove_force("gravity");
		eo.apply_force("gravity", Force(this->get_gravity()));
	}
}

void World::set_spawn_point(Vector3F spawn_point)
{
	this->spawn_point = spawn_point;
}

void World::set_spawn_orientation(Vector3F spawn_orientation)
{
	this->spawn_orientation = spawn_orientation;
}

void World::add_object(Object obj)
{
	this->objects.push_back(std::move(obj));
}

void World::add_entity(Entity ent)
{
	if(ent.get_forces().find("gravity") != ent.get_forces().end())
	{
		ent.remove_force("gravity");
	}
	ent.apply_force("gravity", Force(this->get_gravity()));
	this->entities.push_back(std::move(ent));
}

void World::add_entity_object(EntityObject eo)
{
	if(eo.get_forces().find("gravity") != eo.get_forces().end())
	{
		eo.remove_force("gravity");
	}
	eo.apply_force("gravity", Force(this->get_gravity()));
	this->entity_objects.push_back(std::move(eo));
}

void World::add_light(Light light, GLuint shader_program_handle)
{
	while(this->base_lights.size() >= World::MAXIMUM_LIGHTS)
		this->base_lights.erase(this->base_lights.begin());
	this->base_lights[light.get_uniforms(shader_program_handle, this->base_lights.size())] = light;
}

const std::vector<Object>& World::get_objects() const
{
	return this->objects;
}

const std::vector<Entity>& World::get_entities() const
{
	return this->entities;
}

const std::vector<EntityObject>& World::get_entity_objects() const
{
	return this->entity_objects;
}

std::size_t World::get_size() const
{
	return this->objects.size() + this->entities.size() + this->entity_objects.size();
}

const std::map<std::array<GLint, tz::graphics::light_number_of_uniforms>, Light>& World::get_lights() const
{
	return this->base_lights;
}


void World::kill_lights()
{
	for(auto& iter : this->base_lights)
	{
		// Kill all the lights in the shader before losing all the data by zeroing their corresponding uniforms
		std::vector<float> pos({0.0f, 0.0f, 0.0f}), colour({0.0f, 0.0f, 0.0f});
		glUniform3fv(iter.first.front(), 1, &(pos[0]));
		glUniform3fv(iter.first[1], 1, &(colour[0]));
		glUniform1f(iter.first[2], 0);
		glUniform1f(iter.first[3], 0);
		glUniform1f(iter.first[4], 0);
	}
}

void World::export_world(const std::string& world_link) const
{
	const tz::data::Manager data_manager(this->resources_path);
	MDLF output = MDLF(RawFile(world_link));
	output.get_raw_file().clear();
	std::vector<std::string> object_list;
	std::vector<std::string> entity_object_list;
	output.delete_sequence("objects");
	output.delete_sequence("entityobjects");
	
	output.edit_tag("gravity", tz::util::string::format(tz::util::string::devectorise_list_3<float>(this->gravity)));
	output.edit_tag("spawnpoint", tz::util::string::format(tz::util::string::devectorise_list_3<float>(this->spawn_point)));
	output.edit_tag("spawnorientation", tz::util::string::format(tz::util::string::devectorise_list_3<float>(this->spawn_orientation)));
	for(std::size_t i = 0; i < this->objects.size(); i++)
	{
		const std::string object_name = "object" + tz::util::cast::to_string<float>(i);
		object_list.push_back(object_name);
		const Object current_object = this->objects[i];
		
		output.edit_tag(object_name + ".mesh", data_manager.resource_name(current_object.get_mesh_link()));
		for(auto& texture : current_object.get_textures())
		{
			output.edit_tag(object_name + ".texture" + tz::util::cast::to_string<unsigned int>(static_cast<unsigned int>(texture.second)), data_manager.resource_name(texture.first));
		}
		output.edit_tag(object_name + ".pos", tz::util::string::format(tz::util::string::devectorise_list_3<float>(current_object.get_position())));
		output.edit_tag(object_name + ".rot", tz::util::string::format(tz::util::string::devectorise_list_3<float>(current_object.get_rotation())));
		output.edit_tag(object_name + ".scale", tz::util::string::format(tz::util::string::devectorise_list_3<float>(current_object.get_scale())));
		output.edit_tag(object_name + ".shininess", tz::util::cast::to_string(current_object.get_shininess()));
		output.edit_tag(object_name + ".parallax_map_scale", tz::util::cast::to_string(current_object.get_parallax_map_scale()));
		output.edit_tag(object_name + ".parallax_map_offset", tz::util::cast::to_string(current_object.get_parallax_map_offset()));
		output.edit_tag(object_name + ".displacement_factor", tz::util::cast::to_string(current_object.get_displacement_factor()));
	}
	for(std::size_t i = 0; i < this->entity_objects.size(); i++)
	{
		const std::string entity_object_name = "eo" + tz::util::cast::to_string<float>(i);
		entity_object_list.push_back(entity_object_name);
		const EntityObject current_entity_object = this->entity_objects[i];

		output.edit_tag(entity_object_name + ".mesh", data_manager.resource_name(current_entity_object.get_mesh_link()));
		for(auto& texture : current_entity_object.get_textures())
		{
			output.edit_tag(entity_object_name + ".texture" + tz::util::cast::to_string<unsigned int>(static_cast<unsigned int>(texture.second)), data_manager.resource_name(texture.first));
		}
		output.edit_tag(entity_object_name + ".mass", tz::util::cast::to_string<float>(current_entity_object.get_mass()));
		output.edit_tag(entity_object_name + ".pos", tz::util::string::format(tz::util::string::devectorise_list_3<float>(current_entity_object.get_position())));
		output.edit_tag(entity_object_name + ".rot", tz::util::string::format(tz::util::string::devectorise_list_3<float>(current_entity_object.get_rotation())));
		output.edit_tag(entity_object_name + ".scale", tz::util::string::format(tz::util::string::devectorise_list_3<float>(current_entity_object.get_scale())));
		output.edit_tag(entity_object_name + ".shininess", tz::util::cast::to_string(current_entity_object.get_shininess()));
		output.edit_tag(entity_object_name + ".parallax_map_scale", tz::util::cast::to_string(current_entity_object.get_parallax_map_scale()));
		output.edit_tag(entity_object_name + ".parallax_map_offset", tz::util::cast::to_string(current_entity_object.get_parallax_map_offset()));
		output.edit_tag(entity_object_name + ".displacement_factor", tz::util::cast::to_string(current_entity_object.get_displacement_factor()));
	}
	output.add_sequence("objects", object_list);
	output.add_sequence("entityobjects", entity_object_list);
}

void World::save() const
{
	this->export_world(this->get_file_name());
}

void World::render(Camera& cam, Shader& shader, unsigned int widata_managerh, unsigned int height, const std::vector<std::unique_ptr<Mesh>>& all_meshes, const std::vector<std::unique_ptr<Texture>>& all_textures, const std::vector<std::unique_ptr<NormalMap>>& all_normalmaps, const std::vector<std::unique_ptr<ParallaxMap>>& all_parallaxmaps, const std::vector<std::unique_ptr<DisplacementMap>>& all_displacementmaps)
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH_CLAMP);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	for(auto& obj : this->objects)
	{
		Mesh* mesh = tz::graphics::find_mesh(obj.get_mesh_link(), all_meshes);
		Texture* tex = nullptr; NormalMap* nm = nullptr; ParallaxMap* pm = nullptr; DisplacementMap* dm = nullptr;
		for(auto& texture : obj.get_textures())
		{
			if(texture.second == Texture::TextureType::TEXTURE)
				tex = Texture::get_from_link<Texture>(texture.first, all_textures);
			else if(texture.second == Texture::TextureType::NORMAL_MAP)
				nm = Texture::get_from_link<NormalMap>(texture.first, all_normalmaps);
			else if(texture.second == Texture::TextureType::PARALLAX_MAP)
				pm = Texture::get_from_link<ParallaxMap>(texture.first, all_parallaxmaps);
			else if(texture.second == Texture::TextureType::DISPLACEMENT_MAP)
				dm = Texture::get_from_link<DisplacementMap>(texture.first, all_displacementmaps);
		}
		obj.render(mesh, tex, nm, pm, dm, cam, shader, widata_managerh, height);
	}
	for(auto& eo : this->entity_objects)
	{
		Mesh* mesh = tz::graphics::find_mesh(eo.get_mesh_link(), all_meshes);
		Texture* tex = nullptr; NormalMap* nm = nullptr; ParallaxMap* pm = nullptr; DisplacementMap* dm = nullptr;
		for(auto& texture : eo.get_textures())
		{
			if(texture.second == Texture::TextureType::TEXTURE)
				tex = Texture::get_from_link<Texture>(texture.first, all_textures);
			else if(texture.second == Texture::TextureType::NORMAL_MAP)
				nm = Texture::get_from_link<NormalMap>(texture.first, all_normalmaps);
			else if(texture.second == Texture::TextureType::PARALLAX_MAP)
				pm = Texture::get_from_link<ParallaxMap>(texture.first, all_parallaxmaps);
			else if(texture.second == Texture::TextureType::DISPLACEMENT_MAP)
				dm = Texture::get_from_link<DisplacementMap>(texture.first, all_displacementmaps);
		}
		eo.render(mesh, tex, nm, pm, dm, cam, shader, widata_managerh, height);
	}	
	for(auto& iter : this->base_lights)
	{
		Light light = iter.second;
		std::vector<float> pos, colour;
		pos.push_back(light.get_position().get_x());
		pos.push_back(light.get_position().get_y());
		pos.push_back(light.get_position().get_z());
		colour.push_back(light.get_colour().get_x());
		colour.push_back(light.get_colour().get_y());
		colour.push_back(light.get_colour().get_z());
		glUniform3fv(iter.first.front(), 1, &(pos[0]));
		glUniform3fv(iter.first[1], 1, &(colour[0]));
		glUniform1f(iter.first[2], light.get_power());
		glUniform1f(iter.first[3], light.get_diffuse_component());
		glUniform1f(iter.first[4], light.get_specular_component());
	}
}

void World::update(unsigned int tps)
{
	for(auto& eo : this->entity_objects)
		eo.update_motion(tps);
	for(auto& ent : this->entities)
		ent.update_motion(tps);
}

Object World::retrieve_object_data(const std::string& object_name, std::string resources_path, MDLF& mdlf)
{
	std::string mesh_name = mdlf.get_tag(object_name + ".mesh");
	std::string position_string = mdlf.get_tag(object_name + ".pos");
	std::string rotation_string = mdlf.get_tag(object_name + ".rot");
	std::string scale_string = mdlf.get_tag(object_name + ".scale");
	unsigned int shininess = tz::util::cast::from_string<unsigned int>(mdlf.get_tag(object_name + ".shininess"));
	float parallax_map_scale = tz::util::cast::from_string<float>(mdlf.get_tag(object_name + ".parallax_map_scale"));
	float parallax_map_offset = tz::util::cast::from_string<float>(mdlf.get_tag(object_name + ".parallax_map_offset"));
	float displacement_factor = tz::util::cast::from_string<float>(mdlf.get_tag(object_name + ".displacement_factor"));
	if(!mdlf.exists_tag(object_name + ".shininess"))
		shininess = tz::graphics::default_shininess;
	if(!mdlf.exists_tag(object_name + ".parallax_map_scale"))
		parallax_map_scale = tz::graphics::default_parallax_map_scale;
	if(!mdlf.exists_tag(object_name + ".parallax_map_offset"))
		parallax_map_offset = tz::graphics::default_parallax_map_offset;
	if(!mdlf.exists_tag(object_name + ".displacement_factor"))
		displacement_factor = tz::graphics::default_displacement_factor;
	tz::data::Manager data_manager(resources_path);
	
	std::string mesh_link = data_manager.resource_link(mesh_name);
	std::vector<std::pair<std::string, Texture::TextureType>> textures;
	for(unsigned int i = 0; i < static_cast<unsigned int>(Texture::TextureType::TEXTURE_TYPES); i++)
	{
		std::string texture_name = mdlf.get_tag(object_name + ".texture" + tz::util::cast::to_string(i));
		std::string texture_link = data_manager.resource_link(texture_name);
		textures.emplace_back(texture_link, static_cast<Texture::TextureType>(i));
	}
	return {mesh_link, textures, tz::util::string::vectorise_list_3<float>(tz::util::string::deformat(position_string)), tz::util::string::vectorise_list_3<float>(tz::util::string::deformat(rotation_string)), tz::util::string::vectorise_list_3<float>(tz::util::string::deformat(scale_string)), shininess, parallax_map_scale, parallax_map_offset, displacement_factor};
}

EntityObject World::retrieve_entity_object_data(const std::string& entity_object_name, std::string resources_path, MDLF& mdlf)
{
	// really repeating myself here. bit ugly.
	std::string mesh_name = mdlf.get_tag(entity_object_name + ".mesh");
	std::string mass_string = mdlf.get_tag(entity_object_name + ".mass");
	std::string position_string = mdlf.get_tag(entity_object_name + ".pos");
	std::string rotation_string = mdlf.get_tag(entity_object_name + ".rot");
	std::string scale_string = mdlf.get_tag(entity_object_name + ".scale");
	float mass = tz::util::cast::from_string<float>(mass_string);
	unsigned int shininess = tz::util::cast::from_string<unsigned int>(mdlf.get_tag(entity_object_name + ".shininess"));
	float parallax_map_scale = tz::util::cast::from_string<float>(mdlf.get_tag(entity_object_name + ".parallax_map_scale"));
	float parallax_map_offset = tz::util::cast::from_string<float>(mdlf.get_tag(entity_object_name + ".parallax_map_offset"));
	float displacement_factor = tz::util::cast::from_string<float>(mdlf.get_tag(entity_object_name + ".displacement_factor"));
	if(!mdlf.exists_tag(entity_object_name + ".mass"))
		mass = tz::physics::default_mass;
	if(!mdlf.exists_tag(entity_object_name + ".shininess"))
		shininess = tz::graphics::default_shininess;
	if(!mdlf.exists_tag(entity_object_name + ".parallax_map_scale"))
		parallax_map_scale = tz::graphics::default_parallax_map_scale;
	if(!mdlf.exists_tag(entity_object_name + ".parallax_map_offset"))
		parallax_map_offset = tz::graphics::default_parallax_map_offset;
	if(!mdlf.exists_tag(entity_object_name + ".displacement_factor"))
		displacement_factor = tz::graphics::default_displacement_factor;
	
	tz::data::Manager data_manager(resources_path);
	
	std::string mesh_link = data_manager.resource_link(mesh_name);
	std::vector<std::pair<std::string, Texture::TextureType>> textures;
	for(unsigned int i = 0; i < static_cast<unsigned int>(Texture::TextureType::TEXTURE_TYPES); i++)
	{
		std::string texture_name = mdlf.get_tag(entity_object_name + ".texture" + tz::util::cast::to_string(i));
		std::string texture_link = data_manager.resource_link(texture_name);
		textures.emplace_back(texture_link, static_cast<Texture::TextureType>(i));
	}
	
	return {mesh_link, textures, mass, tz::util::string::vectorise_list_3<float>(tz::util::string::deformat(position_string)), tz::util::string::vectorise_list_3<float>(tz::util::string::deformat(rotation_string)), tz::util::string::vectorise_list_3<float>(tz::util::string::deformat(scale_string)), shininess, parallax_map_scale, parallax_map_offset, displacement_factor};
}