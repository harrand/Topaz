#include "tz/gl/impl/common/shader.hpp"

namespace tz::gl
{
	ShaderInfo::ShaderInfo()
	{
		std::fill(this->source_data.begin(), this->source_data.end(), "");
	}

	void ShaderInfo::set_shader(shader_stage stage, std::string_view source)
	{
		this->source_data[static_cast<int>(stage)] = source;
	}

	std::string_view ShaderInfo::get_shader(shader_stage stage) const
	{
		return this->source_data[static_cast<int>(stage)];
	}

	bool ShaderInfo::has_shader(shader_stage stage) const
	{
		return !this->get_shader(stage).empty();
	}

	void ShaderInfo::set_meta(shader_stage stage, shader_meta meta)
	{
		this->meta_data[static_cast<int>(stage)] = meta;
	}

	const shader_meta& ShaderInfo::get_meta(shader_stage stage) const
	{
		return this->meta_data[static_cast<int>(stage)];
	}
}
