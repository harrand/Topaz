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
Texture::Texture(Bitmap<Pixel> pixel_data): Texture(pixel_data.width, pixel_data.height, false, tz::graphics::TextureComponent::COLOUR_TEXTURE)
{
	// If the pixel type is actually a depth pixel, then we're making a depth texture and need to do it properly.
	if constexpr(std::is_same_v<Pixel, PixelDepth>)
	{
		this->texture_component = tz::graphics::TextureComponent::DEPTH_TEXTURE;
	}
	// handle not generated, do it.
	// Generates a new texture, and just fills it with zeroes if specified.
	glGenTextures(1, &(this->texture_handle));
	glBindTexture(GL_TEXTURE_2D, this->texture_handle);
	std::vector<std::remove_reference_t<decltype(std::declval<Pixel>().data.underlying_data[0])>> image_data;
	for(const auto& pixel : pixel_data.pixels)
	{
		image_data.push_back(pixel.data.underlying_data[0]);
		image_data.push_back(pixel.data.underlying_data[1]);
		image_data.push_back(pixel.data.underlying_data[2]);
		image_data.push_back(pixel.data.underlying_data[3]);
	}
	switch(this->texture_component)
	{
		default:
		case tz::graphics::TextureComponent::COLOUR_TEXTURE:
			this->components = 4;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data.data());
			break;
		case tz::graphics::TextureComponent::DEPTH_TEXTURE:
			this->components = 1;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, this->width, this->height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, image_data.data());
			break;
	}
	/*
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data.data());
	 */
	// Unbind the texture.
	glBindTexture(GL_TEXTURE_2D, 0);
}

template<typename Engine, typename EngineResultType>
DisplacementMap tz::graphics::height_map::generate_random_noise(int width, int height, float displacement_factor, Random<Engine, EngineResultType> noise_function)
{
    std::vector<PixelDepth> pixels;
    pixels.resize(static_cast<std::size_t>(width * height), PixelDepth{0.0f});
    for(auto& pixel : pixels)
        pixel.data.underlying_data = {noise_function.next_float(-1.0f, 1.0f)};
    return {Bitmap<PixelDepth>{pixels, width, height}, displacement_factor};
}