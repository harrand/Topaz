#ifndef TOPAZ_GL2_IMPL_COMMON_SHADER_HPP
#define TOPAZ_GL2_IMPL_COMMON_SHADER_HPP
#include "tz/gl/api/shader.hpp"
#include <string>
#include <array>

namespace tz::gl
{
	class ShaderInfo
	{
	public:
		ShaderInfo();

		void set_shader(shader_stage stage, std::string_view source);
		std::string_view get_shader(shader_stage stage) const;
		bool has_shader(shader_stage stage) const;

		void set_meta(shader_stage stage, shader_meta meta);
		const shader_meta& get_meta(shader_stage stage) const;
	private:
		std::array<std::string, static_cast<int>(shader_stage::Count)> source_data;
		std::array<shader_meta, static_cast<int>(shader_stage::Count)> meta_data;
	};

	static_assert(shader_info_type<ShaderInfo>);
}
#endif // TOPAZ_GL2_IMPL_COMMON_SHADER_HPP
