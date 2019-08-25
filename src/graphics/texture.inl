namespace tz::graphics::asset
{
	inline void unbind_texture()
	{
		glActiveTexture(GL_TEXTURE0 + tz::consts::graphics::shader::sampler::texture_id);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	inline void unbind_normal_map(Shader& render_shader)
	{
		glActiveTexture(GL_TEXTURE0 + tz::consts::graphics::shader::sampler::normal_map_id);
		glBindTexture(GL_TEXTURE_2D, 0);
		render_shader.set_uniform<bool>("has_normal_map", false);
	}

	inline void unbind_parallax_map(Shader& render_shader)
	{
		glActiveTexture(GL_TEXTURE0 + tz::consts::graphics::shader::sampler::parallax_map_id);
		glBindTexture(GL_TEXTURE_2D, 0);
		render_shader.set_uniform<bool>("has_parallax_map", false);
	}

	inline void unbind_displacement_map(Shader& render_shader)
	{
		glActiveTexture(GL_TEXTURE0 + tz::consts::graphics::shader::sampler::displacement_map_id);
		glBindTexture(GL_TEXTURE_2D, 0);
		render_shader.set_uniform<bool>("has_displacement_map", false);
	}

	inline void unbind_specular_map(Shader& render_shader)
	{
		glActiveTexture(GL_TEXTURE0 + tz::consts::graphics::shader::sampler::specular_map_id);
		glBindTexture(GL_TEXTURE_2D, 0);
		render_shader.set_uniform<bool>("has_specular_map", false);
	}

	inline void unbind_emissive_map(Shader& render_shader)
	{
		glActiveTexture(GL_TEXTURE0 + tz::consts::graphics::shader::sampler::emissive_map_id);
		glBindTexture(GL_TEXTURE_2D, 0);
		render_shader.set_uniform<bool>("has_emissive_map", false);
	}

	inline void unbind_all_textures(Shader& render_shader)
	{
		unbind_texture();
		unbind_normal_map(render_shader);
		unbind_parallax_map(render_shader);
		unbind_displacement_map(render_shader);
		unbind_specular_map(render_shader);
		unbind_emissive_map(render_shader);
	}

	inline void unbind_extra_texture(Shader& render_shader, std::size_t extra_shader_id)
	{
		glActiveTexture(GL_TEXTURE0 + tz::consts::graphics::shader::sampler::initial_extra_texture_id);
		glBindTexture(GL_TEXTURE_2D, 0);
		render_shader.set_uniform<bool>(std::string("extra_texture") + std::to_string(extra_shader_id) + "_exists", false);
	}

	inline void unbind_all_extra_textures(Shader& render_shader)
	{
		std::size_t id = tz::consts::graphics::shader::sampler::initial_extra_texture_id;
		for(std::size_t i = 0; i < 8; i++)
		{
			tz::graphics::asset::unbind_extra_texture(render_shader, id++);
		}
	}
}

/*
template<typename Engine, typename EngineResultType>
DisplacementMap tz::graphics::height_map::generate_random_noise(int width, int height, float displacement_factor, Random<Engine, EngineResultType> noise_function)
{
	std::vector<PixelDepth> pixels;
	pixels.resize(static_cast<std::size_t>(width * height), PixelDepth{0.0f});
	for(auto& pixel : pixels)
		pixel.data.underlying_data = {noise_function.next_float(-1.0f, 1.0f)};
	return {Bitmap<PixelDepth>{pixels, width, height}, displacement_factor};
}
*/