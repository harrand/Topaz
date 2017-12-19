#include "world.hpp"
#include "data.hpp"

World::World(std::string filename, std::string resources_path, const std::vector<std::unique_ptr<Mesh>>& all_meshes, const std::vector<std::unique_ptr<Texture>>& all_textures, const std::vector<std::unique_ptr<NormalMap>>& all_normal_maps, const std::vector<std::unique_ptr<ParallaxMap>>& all_parallax_maps, const std::vector<std::unique_ptr<DisplacementMap>>& all_displacement_maps): filename(std::move(filename)), resources_path(std::move(resources_path))
{
	MDLF input(RawFile(this->filename));
	std::string spawn_point_string = input.get_tag("spawnpoint"), spawn_orientation_string = input.get_tag("spawnorientation"), gravity_string = input.get_tag("gravity");
	// Initialise spawn_point, spawn_orientation and gravity to the values specified in filename MDL file. If no such tags could be found & validated, zero them.
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
	// Parse all objects and entity_objects, and add them to the data vectors.
	std::vector<std::string> object_list = input.get_sequence("objects");
	std::vector<std::string> entity_object_list = input.get_sequence("entityobjects");
	for(std::string object_name : object_list)
	{
		this->add_object(World::retrieve_object_data(object_name, this->resources_path, input, all_meshes, all_textures, all_normal_maps, all_parallax_maps, all_displacement_maps));
	}
	for(std::string entity_object_name : entity_object_list)
		this->add_entity_object(World::retrieve_entity_object_data(entity_object_name, this->resources_path, input, all_meshes, all_textures, all_normal_maps, all_parallax_maps, all_displacement_maps));
}

World::World(const World& copy): World(copy.filename){}

World::World(World&& move): spawn_point(move.spawn_point), spawn_orientation(move.spawn_orientation), filename(move.filename), resources_path(move.resources_path), gravity(move.gravity), objects(move.objects), entities(move.entities), entity_objects(std::move(move.entity_objects)), base_lights(std::move(move.base_lights)){}

const std::string& World::get_file_name() const
{
	return this->filename;
}

const Vector3F& World::get_gravity() const
{
	return this->gravity;
}

// Gravity is a force, and must be handled like any other force would. Essentially updates the 'gravity' force in all entities and entity_objects so that they're affected by the new force. It is done like this so that Entity and EntityObject require no reference to World whatsoever.
void World::set_gravity(Vector3F gravity)
{
	this->gravity = gravity;
	for(Entity& ent : this->entities)
	{
		// Both of these are O(n) Ω(1) ϴ(1), where n = number of existing forces
		ent.remove_force("gravity");
		ent.apply_force("gravity", Force(this->get_gravity()));
	}
	for(EntityObject& eo : this->entity_objects)
	{
		// Both once again O(n) Ω(1) ϴ(1), where n = number of existing forces
		eo.remove_force("gravity");
		eo.apply_force("gravity", Force(this->get_gravity()));
	}
}

void World::add_object(Object obj)
{
	this->objects.push_back(std::move(obj));
}

// Add a copy of an entity into this world. It will have this world's gravity instantly applied to it.
void World::add_entity(Entity ent)
{
	// Once we add an entity, make sure the copy has the correct gravity force applying on it.
	// std::unordered_map::find is O(n) Ω(1) ϴ(1), where n = number of elements
	if(ent.get_forces().find("gravity") != ent.get_forces().end())
	{
		// Entity::remove_force is O(n) Ω(1) ϴ(1), where n = number of existing forces
		ent.remove_force("gravity");
	}
	// Entity::apply_force is O(n) Ω(1) ϴ(1), where n = number of existing forces
	ent.apply_force("gravity", Force(this->get_gravity()));
	// O(1) amortised Ω(1) ϴ(1) amortised
	this->entities.push_back(std::move(ent));
}

// See documentation for World::add_entity(Entity).
void World::add_entity_object(EntityObject eo)
{
	if(eo.get_forces().find("gravity") != eo.get_forces().end())
	{
		eo.remove_force("gravity");
	}
	eo.apply_force("gravity", Force(this->get_gravity()));
	this->entity_objects.push_back(std::move(eo));
}

// Adds a light to this world, with the shader handle of the corresponding shader that should handle such lighting.
void World::add_light(Light light, GLuint shader_program_handle)
{
	// std::map::operator[] is O(log n) where n is size of map
	while(this->base_lights.size() >= tz::graphics::maximum_lights)
		this->base_lights.erase(this->base_lights.begin());
	this->base_lights[light.get_uniforms(shader_program_handle, this->base_lights.size())] = light;
}

void World::remove_object(const Object& obj)
{
	this->objects.erase(std::remove(this->objects.begin(), this->objects.end(), obj), this->objects.end());
}

void World::remove_entity(const Entity& ent)
{
	this->entities.erase(std::remove(this->entities.begin(), this->entities.end(), ent), this->entities.end());
}

void World::remove_entity_object(const EntityObject& eo)
{
	this->entity_objects.erase(std::remove(this->entity_objects.begin(), this->entity_objects.end(), eo), this->entity_objects.end());
}

void World::remove_light(const Light& light)
{
	for(auto it : this->base_lights)
		if(it.second == light)
			this->base_lights.erase(it.first);
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
		
		output.edit_tag(object_name + ".mesh", data_manager.resource_name(current_object.get_mesh().get_file_name()));
		for(auto& texture : current_object.get_textures())
		{
			output.edit_tag(object_name + ".texture" + tz::util::cast::to_string<unsigned int>(static_cast<unsigned int>(texture.first)), data_manager.resource_name(texture.second->get_file_name()));
		}
		output.edit_tag(object_name + ".pos", tz::util::string::format(tz::util::string::devectorise_list_3<float>(current_object.position)));
		output.edit_tag(object_name + ".rot", tz::util::string::format(tz::util::string::devectorise_list_3<float>(current_object.rotation)));
		output.edit_tag(object_name + ".scale", tz::util::string::format(tz::util::string::devectorise_list_3<float>(current_object.scale)));
		output.edit_tag(object_name + ".shininess", tz::util::cast::to_string(current_object.shininess));
		output.edit_tag(object_name + ".parallax_map_scale", tz::util::cast::to_string(current_object.parallax_map_scale));
		output.edit_tag(object_name + ".parallax_map_offset", tz::util::cast::to_string(current_object.parallax_map_offset));
		output.edit_tag(object_name + ".displacement_factor", tz::util::cast::to_string(current_object.displacement_factor));
	}
	for(std::size_t i = 0; i < this->entity_objects.size(); i++)
	{
		const std::string entity_object_name = "eo" + tz::util::cast::to_string<float>(i);
		entity_object_list.push_back(entity_object_name);
		const EntityObject current_entity_object = this->entity_objects[i];

		output.edit_tag(entity_object_name + ".mesh", data_manager.resource_name(current_entity_object.get_mesh().get_file_name()));
		for(auto& texture : current_entity_object.get_textures())
		{
			output.edit_tag(entity_object_name + ".texture" + tz::util::cast::to_string<unsigned int>(static_cast<unsigned int>(texture.first)), data_manager.resource_name(texture.second->get_file_name()));
		}
		output.edit_tag(entity_object_name + ".mass", tz::util::cast::to_string(current_entity_object.mass));
		output.edit_tag(entity_object_name + ".pos", tz::util::string::format(tz::util::string::devectorise_list_3<float>(current_entity_object.position)));
		output.edit_tag(entity_object_name + ".rot", tz::util::string::format(tz::util::string::devectorise_list_3<float>(current_entity_object.rotation)));
		output.edit_tag(entity_object_name + ".scale", tz::util::string::format(tz::util::string::devectorise_list_3<float>(current_entity_object.scale)));
		output.edit_tag(entity_object_name + ".shininess", tz::util::cast::to_string(current_entity_object.shininess));
		output.edit_tag(entity_object_name + ".parallax_map_scale", tz::util::cast::to_string(current_entity_object.parallax_map_scale));
		output.edit_tag(entity_object_name + ".parallax_map_offset", tz::util::cast::to_string(current_entity_object.parallax_map_offset));
		output.edit_tag(entity_object_name + ".displacement_factor", tz::util::cast::to_string(current_entity_object.displacement_factor));
	}
	output.add_sequence("objects", object_list);
	output.add_sequence("entityobjects", entity_object_list);
}

void World::save() const
{
	this->export_world(this->get_file_name());
}

void World::render(const Camera& cam, Shader* shader, unsigned int width, unsigned int height)
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH_CLAMP);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	// If any objects have been added or removed since last render call, update the instancified list of objects before rendering them.
	if(this->objects.size() != this->total_instances())
		this->update_instances();
	for(auto& object : this->instancified_objects)
		object.render(cam, shader, width, height);
	for(auto& entity_object : this->entity_objects)
		entity_object.render(cam, shader, width, height);
	for(auto& iter : this->base_lights)
	{
		Light light = iter.second;
		std::vector<float> pos, colour;
		pos.push_back(light.position.x);
		pos.push_back(light.position.y);
		pos.push_back(light.position.z);
		colour.push_back(light.colour.x);
		colour.push_back(light.colour.y);
		colour.push_back(light.colour.z);
		glUniform3fv(iter.first.front(), 1, &(pos[0]));
		glUniform3fv(iter.first[1], 1, &(colour[0]));
		glUniform1f(iter.first[2], light.power);
		glUniform1f(iter.first[3], light.diffuse_component);
		glUniform1f(iter.first[4], light.specular_component);
	}
}

void World::update(unsigned int tps)
{
	for(auto& eo : this->entity_objects)
		eo.update_motion(tps);
	for(auto& ent : this->entities)
		ent.update_motion(tps);
}

Object World::retrieve_object_data(const std::string& object_name, std::string resources_path, MDLF& mdlf, const std::vector<std::unique_ptr<Mesh>>& all_meshes, const std::vector<std::unique_ptr<Texture>>& all_textures, const std::vector<std::unique_ptr<NormalMap>>& all_normal_maps, const std::vector<std::unique_ptr<ParallaxMap>>& all_parallax_maps, const std::vector<std::unique_ptr<DisplacementMap>>& all_displacement_maps)
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
	std::map<tz::graphics::TextureType, Texture*> textures;
	for(unsigned int i = 0; i < static_cast<unsigned int>(tz::graphics::TextureType::TEXTURE_TYPES); i++)
	{
		std::string texture_name = mdlf.get_tag(object_name + ".texture" + tz::util::cast::to_string(i));
		std::string texture_link = data_manager.resource_link(texture_name);
		Texture* tex = nullptr;
		switch(static_cast<tz::graphics::TextureType>(i))
		{
			case tz::graphics::TextureType::TEXTURE:
			default:
				tex = Texture::get_from_link<Texture>(texture_link, all_textures);
				break;
			case tz::graphics::TextureType::NORMAL_MAP:
				tex = Texture::get_from_link<NormalMap>(texture_link, all_normal_maps);
				break;
			case tz::graphics::TextureType::PARALLAX_MAP:
				tex = Texture::get_from_link<ParallaxMap>(texture_link, all_parallax_maps);
				break;
			case tz::graphics::TextureType::DISPLACEMENT_MAP:
				tex = Texture::get_from_link<DisplacementMap>(texture_link, all_displacement_maps);
				break;
		}
		textures.emplace(static_cast<tz::graphics::TextureType>(i), tex);
	}
	return {tz::graphics::find_mesh(mesh_link, all_meshes), textures, tz::util::string::vectorise_list_3<float>(tz::util::string::deformat(position_string)), tz::util::string::vectorise_list_3<float>(tz::util::string::deformat(rotation_string)), tz::util::string::vectorise_list_3<float>(tz::util::string::deformat(scale_string)), shininess, parallax_map_scale, parallax_map_offset, displacement_factor};
}

EntityObject World::retrieve_entity_object_data(const std::string& entity_object_name, std::string resources_path, MDLF& mdlf, const std::vector<std::unique_ptr<Mesh>>& all_meshes, const std::vector<std::unique_ptr<Texture>>& all_textures, const std::vector<std::unique_ptr<NormalMap>>& all_normal_maps, const std::vector<std::unique_ptr<ParallaxMap>>& all_parallax_maps, const std::vector<std::unique_ptr<DisplacementMap>>& all_displacement_maps)
{
	// No point repeating code from World::retrieve_object_data, so call it to receive a valid Object. Then parse the mass from the data-file, and cobble all the data together to create the final EntityObject. This doesn't really waste memory as Objects are now smaller than before, as pointers << strings.
	Object object = World::retrieve_object_data(entity_object_name, resources_path, mdlf, all_meshes, all_textures, all_normal_maps, all_parallax_maps, all_displacement_maps);
	std::string mass_string = mdlf.get_tag(entity_object_name + ".mass");
	float mass = tz::util::cast::from_string<float>(mass_string);
	if(!mdlf.exists_tag(entity_object_name + ".mass"))
		mass = tz::physics::default_mass;
	return{&(object.get_mesh()), object.get_textures(), mass, object.position, object.rotation, object.scale, object.shininess, object.parallax_map_scale, object.parallax_map_offset, object.displacement_factor};
}

void World::update_instances()
{
	this->instancified_objects = tz::graphics::instancify_full(this->objects);
}

std::size_t World::total_instances() const
{
	std::size_t quantity = 0;
	for(const Object& object : this->instancified_objects)
		if(tz::graphics::is_instanced(&(object.get_mesh())))
			quantity += dynamic_cast<const InstancedMesh*>(&(object.get_mesh()))->get_instance_quantity();
	return quantity;
}