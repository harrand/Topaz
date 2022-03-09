#include "gl/impl/frontend/common/shader.hpp"

namespace tz::gl
{
	ShaderInfo::ShaderInfo()
	{
		std::fill(this->source_data.begin(), this->source_data.end(), "");
	}

	void ShaderInfo::set_shader(ShaderStage stage, std::string_view source)
	{
		this->source_data[static_cast<int>(stage)] = source;
	}

	std::string_view ShaderInfo::get_shader(ShaderStage stage) const
	{
		return this->source_data[static_cast<int>(stage)];
	}

	bool ShaderInfo::has_shader(ShaderStage stage) const
	{
		return !this->get_shader(stage).empty();
	}

	void ShaderInfo::set_meta(ShaderStage stage, ShaderMeta meta)
	{
		this->meta_data[static_cast<int>(stage)] = meta;
	}

	const ShaderMeta& ShaderInfo::get_meta(ShaderStage stage) const
	{
		return this->meta_data[static_cast<int>(stage)];
	}
}
