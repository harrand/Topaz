namespace tz::graphics::asset
{
	inline void unbind_texture()
	{
		glActiveTexture(GL_TEXTURE0 + tz::graphics::texture_sampler_id);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	inline void unbind_normal_map(Shader& render_shader)
	{
		glActiveTexture(GL_TEXTURE0 + tz::graphics::normal_map_sampler_id);
		glBindTexture(GL_TEXTURE_2D, 0);
		render_shader.set_uniform<bool>("has_normal_map", false);
	}

	inline void unbind_parallax_map(Shader& render_shader)
	{
		glActiveTexture(GL_TEXTURE0 + tz::graphics::parallax_map_sampler_id);
		glBindTexture(GL_TEXTURE_2D, 0);
		render_shader.set_uniform<bool>("has_parallax_map", false);
	}

	inline void unbind_displacement_map(Shader& render_shader)
	{
		glActiveTexture(GL_TEXTURE0 + tz::graphics::displacement_map_sampler_id);
		glBindTexture(GL_TEXTURE_2D, 0);
		render_shader.set_uniform<bool>("has_displacement_map", false);
	}

	inline void unbind_all_textures(Shader& render_shader)
	{
		unbind_texture();
		unbind_normal_map(render_shader);
		unbind_parallax_map(render_shader);
		unbind_displacement_map(render_shader);
	}
}

template<class Pixel>
Texture::Texture(Bitmap<Pixel> pixel_data): Texture(pixel_data.width, pixel_data.height, false)
{
	// handle not generated, do it.
	// Generates a new texture, and just fills it with zeroes if specified.
	glGenTextures(1, &(this->texture_handle));
	glBindTexture(GL_TEXTURE_2D, this->texture_handle);
	std::vector<unsigned char> image_data;
	for(const auto& pixel : pixel_data.pixels)
	{
		image_data.push_back(pixel.data.data[0]);
		image_data.push_back(pixel.data.data[1]);
		image_data.push_back(pixel.data.data[2]);
		image_data.push_back(pixel.data.data[3]);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data.data());
	// Unbind the texture.
	glBindTexture(GL_TEXTURE_2D, 0);
}

template<class T>
T* Texture::get_from_link(const std::string& texture_link, const std::vector<std::unique_ptr<T>>& all_textures)
{
	for(auto& texture : all_textures)
	{
		if(texture->get_file_name() == texture_link)
			return texture.get();
	}
	std::cerr << "Texture link \"" << texture_link << "\" could not be located. Anything using this texture will not render.\n";
	return nullptr;
}