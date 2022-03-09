#ifndef TOPAZ_GL2_IMPL_COMMON_SHADER_HPP
#define TOPAZ_GL2_IMPL_COMMON_SHADER_HPP
#include "gl/api/shader.hpp"
#include <string>
#include <array>

namespace tz::gl
{
	class ShaderInfo
	{
	public:
		ShaderInfo();

		void set_shader(ShaderStage stage, std::string_view source);
		std::string_view get_shader(ShaderStage stage) const;
		bool has_shader(ShaderStage stage) const;

		void set_meta(ShaderStage stage, ShaderMeta meta);
		const ShaderMeta& get_meta(ShaderStage stage) const;
	private:
		std::array<std::string, static_cast<int>(ShaderStage::Count)> source_data;
		std::array<ShaderMeta, static_cast<int>(ShaderStage::Count)> meta_data;
	};

	static_assert(ShaderInfoType<ShaderInfo>);
}
#endif // TOPAZ_GL2_IMPL_COMMON_SHADER_HPP
