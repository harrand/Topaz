template <template <typename> class Collection>
Object3D tz::graphics::instancify(const Collection<Object3D>& objects)
{
	std::vector<Vector3F> positions, rotations, scales;
	Vector3F original_position = objects.front().position;
	Vector3F original_rotation = objects.front().rotation;
	Vector3F original_scale = objects.front().scale;
	for(const Object3D& object : objects)
	{
		positions.push_back(object.position - original_position);
		rotations.push_back(object.rotation - original_rotation);
		scales.push_back(object.scale - original_scale);
	}
	return {std::make_shared<InstancedMesh>(objects.front().get_mesh().get_file_name(), positions, rotations, scales), objects.front().get_textures(), original_position, original_rotation, original_scale};
}

template <template <typename> class Collection>
std::vector<Object3D> tz::graphics::instancify_full(const Collection<Object3D>& objects)
{
	using mesh_cref_t = std::reference_wrapper<const Mesh>;
	using textures_cref_t = std::reference_wrapper<const std::map<tz::graphics::TextureType, Texture*>>;
	auto useless_comparator = []([[maybe_unused]] auto lhs, [[maybe_unused]] auto rhs){return true;};
	std::multimap<std::pair<mesh_cref_t, textures_cref_t>, std::size_t, std::function<bool(std::pair<mesh_cref_t, textures_cref_t>, std::pair<mesh_cref_t, textures_cref_t>)>> asset_mappings(useless_comparator);
	// O(n log n)
	for(std::size_t i = 0; i < objects.size(); i++)
	{
		const Object3D& object = objects[i];
		mesh_cref_t mesh_read = std::cref(object.get_mesh());
		textures_cref_t textures_read = std::cref(object.get_textures());
		asset_mappings.emplace(std::make_pair(mesh_read, textures_read), i);
		// std::multimap::emplace is O(log n) where n == std::multimap::size();
	}
	std::vector<Object3D> instancified_objects;
	std::vector<Object3D> duplicates;
	std::size_t maximum_size = objects.size();
	// This will in all but the absolute worst case allocate more memory than required, but it's better to reserve maximum and then shrink_to_fit whilst guaranteeing no extra allocation as opposed to possibly doing it multiple times in a loop.
	instancified_objects.reserve(maximum_size);
	duplicates.reserve(maximum_size);
	std::pair<mesh_cref_t, textures_cref_t> key_cache = asset_mappings.begin()->first;
	bool last_ends_cluster = false;
	for(const auto& pair : asset_mappings)
	{
		if(pair.first.first.get() == key_cache.first.get() && pair.first.second.get() == key_cache.second.get())
		{
			// O(1) amortised
			duplicates.push_back(objects[pair.second]);
			last_ends_cluster = false;
		}
		else
		{
			// O(1) amortised
			key_cache = pair.first;
			instancified_objects.push_back(tz::graphics::instancify(duplicates));
			duplicates.clear();
			last_ends_cluster = true;
			duplicates.push_back(objects[pair.second]);
		}
	}
	if(!last_ends_cluster)
		instancified_objects.push_back(tz::graphics::instancify(duplicates));
	// Could shrink_to_fit the result, but it's really not worth the effort just to trade linear average case time complexity == worst case for average case memory complexity == worst case
	//instancified_objects.shrink_to_fit();
	return instancified_objects;
}