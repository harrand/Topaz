#include "scene.hpp"
#include "data.hpp"

Scene::Scene(): spawn_point(Vector3F()), spawn_orientation(Vector3F()), filename({}), resources_path({}), objects({}), heap_objects({}){}

Scene::Scene(std::string filename, std::string resources_path, const std::vector<std::unique_ptr<Mesh>>& all_meshes, const std::vector<std::unique_ptr<Texture>>& all_textures, const std::vector<std::unique_ptr<NormalMap>>& all_normal_maps, const std::vector<std::unique_ptr<ParallaxMap>>& all_parallax_maps, const std::vector<std::unique_ptr<DisplacementMap>>& all_displacement_maps, bool batch): filename(std::move(filename)), resources_path(std::move(resources_path))
{
	MDLFile input(this->get_file_name());
	std::string spawn_point_string = input.get_tag(tz::scene::spawnpoint_tag_name), spawn_orientation_string = input.get_tag(tz::scene::spawnorientation_tag_name);
	// Initialise spawn_point, spawn_orientation and gravity to the values specified in filename MDL file. If no such tags could be found & validated, zero them.
	if(spawn_point_string != mdl::default_string && spawn_orientation_string != mdl::default_string)
	{
		this->spawn_point = tz::util::string::vectorise_list_3<float>(tz::util::string::deformat(spawn_point_string));
		this->spawn_orientation = tz::util::string::vectorise_list_3<float>(tz::util::string::deformat(spawn_orientation_string));
	}
	else
	{
		this->spawn_point = Vector3F();
		this->spawn_orientation = Vector3F();
	}
	// Parse all objects and entity_objects, and add them to the data vectors.
	std::vector<std::string> object_list = input.get_sequence(tz::scene::objects_sequence_name);
	this->objects.reserve(object_list.size());
	for(const auto& object_name : object_list)
		this->add_object(Scene::retrieve_object_data(object_name, this->resources_path.value(), input, all_meshes, all_textures, all_normal_maps, all_parallax_maps, all_displacement_maps));
	if(batch)
		this->objects = std::move(tz::graphics::batch_full(this->objects));
}

bool Scene::has_file_name() const
{
	return this-filename.has_value();
}

const std::string& Scene::get_file_name() const
{
	return this->filename.value();
}

void Scene::add_object(Object obj)
{
	this->objects.push_back(std::move(obj));
}

void Scene::remove_object(const Object& obj)
{
	this->objects.erase(std::remove(this->objects.begin(), this->objects.end(), obj), this->objects.end());
}

std::vector<std::reference_wrapper<const Object>> Scene::get_objects() const
{
	std::vector<std::reference_wrapper<const Object>> object_crefs;
	for(const Object& object_cref : this->objects)
		object_crefs.push_back(std::cref(object_cref));
	for(const std::shared_ptr<Object>& object_ptr : this->heap_objects)
		object_crefs.push_back(std::cref(*object_ptr));
	return object_crefs;
}

std::size_t Scene::get_size() const
{
	return this->objects.size() + this->heap_objects.size();
}

void Scene::export_scene(const std::string& scene_link) const
{
	if(!this->resources_path.has_value())
	{
		tz::util::log::error("Tried to export scene but the scene had no resources_path attached to it.");
		return;
	}
	const tz::data::Manager data_manager(this->resources_path.value());
	MDLFile output = MDLFile(scene_link);
	output.write("# Topaz Auto-Generated Scene File", true);
	std::vector<std::string> object_list, entity_object_list;
	output.delete_sequence(tz::scene::objects_sequence_name);

	output.edit_tag(tz::scene::spawnpoint_tag_name, tz::util::string::format(tz::util::string::devectorise_list_3<float>(this->spawn_point)));
	output.edit_tag(tz::scene::spawnorientation_tag_name, tz::util::string::format(tz::util::string::devectorise_list_3<float>(this->spawn_orientation)));
	for(std::size_t i = 0; i < this->objects.size(); i++)
	{
		const std::string object_name = tz::scene::object_tag_prefix + tz::util::cast::to_string(i);
		object_list.push_back(object_name);
		const Object current_object = this->objects[i];

		output.edit_tag(object_name + ".mesh", data_manager.resource_name(current_object.get_mesh().get_file_name()));

        if(current_object.get_material().has_texture())
            output.edit_tag(object_name + ".texture0", data_manager.resource_name(current_object.get_material().get_texture()->get_file_name()));
        if(current_object.get_material().has_normal_map())
            output.edit_tag(object_name + ".texture1", data_manager.resource_name(current_object.get_material().get_normal_map()->get_file_name()));
        if(current_object.get_material().has_parallax_map())
            output.edit_tag(object_name + ".texture2", data_manager.resource_name(current_object.get_material().get_parallax_map()->get_file_name()));
        if(current_object.get_material().has_displacement_map())
            output.edit_tag(object_name + ".texture3", data_manager.resource_name(current_object.get_material().get_displacement_map()->get_file_name()));
		output.edit_tag(object_name + ".pos", tz::util::string::format(tz::util::string::devectorise_list_3<float>(current_object.position)));
		output.edit_tag(object_name + ".rot", tz::util::string::format(tz::util::string::devectorise_list_3<float>(current_object.rotation)));
		output.edit_tag(object_name + ".scale", tz::util::string::format(tz::util::string::devectorise_list_3<float>(current_object.scale)));
	}
	output.add_sequence(tz::scene::objects_sequence_name, object_list);
}

void Scene::save() const
{
	this->export_scene(this->get_file_name());
}

void Scene::render(const Camera& cam, Shader* shader, unsigned int width, unsigned int height)
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH_CLAMP);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	for(auto& object : this->objects)
		object.render(cam, shader, width, height);
    for(auto& heap_object : this->heap_objects)
        heap_object->render(cam, shader, width, height);
}
Object Scene::retrieve_object_data(const std::string& object_name, const std::string& resources_path, MDLFile& mdlf, const std::vector<std::unique_ptr<Mesh>>& all_meshes, const std::vector<std::unique_ptr<Texture>>& all_textures, const std::vector<std::unique_ptr<NormalMap>>& all_normal_maps, const std::vector<std::unique_ptr<ParallaxMap>>& all_parallax_maps, const std::vector<std::unique_ptr<DisplacementMap>>& all_displacement_maps)
{
	std::string mesh_name = mdlf.get_tag(object_name + ".mesh");
	std::string position_string = mdlf.get_tag(object_name + ".pos");
	std::string rotation_string = mdlf.get_tag(object_name + ".rot");
	std::string scale_string = mdlf.get_tag(object_name + ".scale");
	tz::data::Manager data_manager(resources_path);
	
	std::string mesh_link = data_manager.resource_link(mesh_name);
    Texture* texture = Texture::get_from_link<Texture>(data_manager.resource_link(mdlf.get_tag(object_name + ".texture0")), all_textures);
    NormalMap* normal_map = Texture::get_from_link<NormalMap>(data_manager.resource_link(mdlf.get_tag(object_name + ".texture1")), all_normal_maps);
    ParallaxMap* parallax_map = Texture::get_from_link<ParallaxMap>(data_manager.resource_link(mdlf.get_tag(object_name + ".texture2")), all_parallax_maps);
    DisplacementMap* displacement_map = Texture::get_from_link<DisplacementMap>(data_manager.resource_link(mdlf.get_tag(object_name + ".texture3")), all_displacement_maps);
    Material material(texture, normal_map, parallax_map, displacement_map);
	return {tz::graphics::find_mesh(mesh_link, all_meshes), material, tz::util::string::vectorise_list_3<float>(tz::util::string::deformat(position_string)), tz::util::string::vectorise_list_3<float>(tz::util::string::deformat(rotation_string)), tz::util::string::vectorise_list_3<float>(tz::util::string::deformat(scale_string))};
}