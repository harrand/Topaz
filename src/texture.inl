template<class T>
T* Texture::get_from_link(const std::string& texture_link, const std::vector<std::unique_ptr<T>>& all_textures)
{
	for(auto& texture : all_textures)
	{
		if(texture->get_file_name() == texture_link)
			return texture.get();
	}
	return nullptr;
}