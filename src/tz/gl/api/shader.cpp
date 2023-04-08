#include "tz/gl/api/shader.hpp"

namespace tz::gl
{
	shader_info::shader_info()
	{
		std::fill(this->source_data.begin(), this->source_data.end(), "");
	}

	void shader_info::set_shader(shader_stage stage, std::string_view source)
	{
		this->source_data[static_cast<int>(stage)] = source;
	}

	std::string_view shader_info::get_shader(shader_stage stage) const
	{
		return this->source_data[static_cast<int>(stage)];
	}

	bool shader_info::has_shader(shader_stage stage) const
	{
		return !this->get_shader(stage).empty();
	}

	void shader_info::set_meta(shader_stage stage, shader_meta meta)
	{
		this->meta_data[static_cast<int>(stage)] = meta;
	}

	const shader_meta& shader_info::get_meta(shader_stage stage) const
	{
		return this->meta_data[static_cast<int>(stage)];
	}
}
