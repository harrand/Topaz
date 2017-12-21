template <template <typename> class Collection>
Object tz::graphics::instancify(const Collection<Object>& objects)
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
	return {std::make_shared<InstancedMesh>(objects.front().get_mesh().get_file_name(), positions, rotations, scales), objects.front().get_textures(), original_position, original_rotation, original_scale};
}

template <template <typename> class Collection>
std::vector<Object> tz::graphics::instancify_full(const Collection<Object>& objects)
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
			last_ends_cluster = false;
		}
		else
		{
			key_cache = pair.first;
			instancified_objects.push_back(tz::graphics::instancify(duplicates));
			duplicates.clear();
			last_ends_cluster = true;
			duplicates.push_back(objects[pair.second]);
		}
	}
	if(!last_ends_cluster)
		instancified_objects.push_back(tz::graphics::instancify(duplicates));
	return instancified_objects;
}