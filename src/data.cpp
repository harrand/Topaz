#include "data.hpp"

AssetBuffer::AssetBuffer(std::vector<std::shared_ptr<Mesh>> meshes, std::vector<std::shared_ptr<Texture>> textures, std::vector<std::shared_ptr<NormalMap>> normal_maps, std::vector<std::shared_ptr<ParallaxMap>> parallax_maps, std::vector<std::shared_ptr<DisplacementMap>> displacement_maps): meshes(meshes), textures(textures), normal_maps(normal_maps), parallax_maps(parallax_maps), displacement_maps(displacement_maps){}

Asset::Asset(std::weak_ptr<Mesh> mesh, std::weak_ptr<Texture> texture, std::weak_ptr<NormalMap> normal_map, std::weak_ptr<ParallaxMap> parallax_map, std::weak_ptr<DisplacementMap> displacement_map): mesh(mesh), texture(texture), normal_map(normal_map), parallax_map(parallax_map), displacement_map(displacement_map){}

bool Asset::valid_mesh() const
{
	return !tz::data::is_uninitialized(this->mesh) && !this->mesh.expired();
}

bool Asset::valid_texture() const
{
	return !tz::data::is_uninitialized(this->texture) && !this->texture.expired();
}

bool Asset::valid_normal_map() const
{
	return !tz::data::is_uninitialized(this->normal_map) && !this->normal_map.expired();
}

bool Asset::valid_parallax_map() const
{
	return !tz::data::is_uninitialized(this->parallax_map) && !this->parallax_map.expired();
}

bool Asset::valid_displacement_map() const
{
	return !tz::data::is_uninitialized(this->displacement_map) && !this->displacement_map.expired();
}

tz::data::Manager::Manager(std::string datafilename): data_file(datafilename) {}

std::string tz::data::Manager::resource_link(const std::string& resource_name) const
{
	// just gets the value of resource_name.path
	return this->data_file.get_tag(resource_name + ".path");
}

std::string tz::data::Manager::resource_name(const std::string& resource_link) const
{
	// basically removes .path from the end of the tag which has resource_link as a value
	std::vector<std::string> lines = this->data_file.get_lines();
	for(std::string& line : lines)
	{
		std::vector<std::string> line_split = tz::util::string::split_string(line, ':');
		if(line_split.size() != 0)
		{
			std::string tagname = line_split.front();
			if(this->data_file.get_tag(tagname) == resource_link)
			{
				std::string undesired_suffix = ".path";
				tagname.erase(tagname.find(undesired_suffix), undesired_suffix.length());
				return tagname;
			}
		}
	}
	return {mdl::default_string};
}

std::unordered_map<std::string, std::string> tz::data::Manager::retrieve_models(const char* sequence_name) const
{
	std::unordered_map<std::string, std::string> model_map; // unordered map <resource.path, resource.name>
	for(std::string& model : this->data_file.get_sequence(sequence_name))
	{
		model_map[this->resource_link(model)] = this->data_file.get_tag(model + ".name");
	}
	return model_map;
}

std::unordered_map<std::string, std::string> tz::data::Manager::retrieve_textures(const char* sequence_name) const
{
	std::unordered_map<std::string, std::string> texture_map; // unordered map <resource.path, resource.name>
	for(const std::string& texture : this->data_file.get_sequence(sequence_name))
	{
		texture_map[this->resource_link(texture)] = this->data_file.get_tag(texture + ".name");
	}
	return texture_map;
}

std::unordered_map<std::string, std::string> tz::data::Manager::retrieve_normal_maps(const char* sequence_name) const
{
	std::unordered_map<std::string, std::string> normalmap_map; // unordered map <resource.path, resource.name>
	for(const std::string& normalmap : this->data_file.get_sequence(sequence_name))
	{
		normalmap_map[this->resource_link(normalmap)] = this->data_file.get_tag(normalmap + ".name");
	}
	return normalmap_map;
}

std::unordered_map<std::string, std::string> tz::data::Manager::retrieve_parallax_maps(const char* sequence_name) const
{
	std::unordered_map<std::string, std::string> parallaxmap_map; // unordered map <resource.path, resource.name>
	for(const std::string& parallaxmap : this->data_file.get_sequence(sequence_name))
	{
		parallaxmap_map[this->resource_link(parallaxmap)] = this->data_file.get_tag(parallaxmap + ".name");
	}
	return parallaxmap_map;
}

std::unordered_map<std::string, std::string> tz::data::Manager::retrieve_displacement_maps(const char* sequence_name) const
{
	std::unordered_map<std::string, std::string> displacementmap_map; // unordered map <resource.path, resource.name>
	for(const std::string& displacementmap : this->data_file.get_sequence(sequence_name))
	{
		displacementmap_map[this->resource_link(displacementmap)] = this->data_file.get_tag(displacementmap + ".name");
	}
	return displacementmap_map;
}

unsigned int tz::data::Manager::retrieve_all_data(std::vector<std::unique_ptr<Mesh>>& all_meshes, std::vector<std::unique_ptr<Texture>>& all_textures, std::vector<std::unique_ptr<NormalMap>>& all_normalmaps, std::vector<std::unique_ptr<ParallaxMap>>& all_parallaxmaps, std::vector<std::unique_ptr<DisplacementMap>>& all_displacementmaps) const
{
	std::unordered_map<std::string, std::string> models = this->retrieve_models(), textures = this->retrieve_textures(), normalmaps = this->retrieve_normal_maps(), parallaxmaps = this->retrieve_parallax_maps(), displacementmaps = this->retrieve_displacement_maps();
	unsigned int data_count = 0;
	for(auto& it : models)
	{
		all_meshes.push_back(std::make_unique<Mesh>(it.first));
		data_count++;
	}
	for(auto& it : textures)
	{
		all_textures.push_back(std::make_unique<Texture>(it.first));
		data_count++;
	}
	for(auto& it : normalmaps)
	{
		all_normalmaps.push_back(std::make_unique<NormalMap>(it.first));
		data_count++;
	}
	for(auto& it : parallaxmaps)
	{
		all_parallaxmaps.push_back(std::make_unique<ParallaxMap>(it.first));
		data_count++;
	}
	for(auto& it : displacementmaps)
	{
		all_displacementmaps.push_back(std::make_unique<DisplacementMap>(it.first));
		data_count++;
	}
	return data_count;
}