#ifndef TOPAZ_GL2_API_SHADER_HPP
#define TOPAZ_GL2_API_SHADER_HPP
#include <string>
#include <string_view>
#include <array>

namespace tz::gl
{
	enum class shader_stage
	{
		vertex,
		fragment,
		compute,
		tessellation_control,
		tessellation_evaluation,

		_count
	};

	struct shader_meta
	{

	};

	class shader_info
	{
	public:
		shader_info();

		void set_shader(shader_stage stage, std::string_view source);
		std::string_view get_shader(shader_stage stage) const;
		bool has_shader(shader_stage stage) const;

		void set_meta(shader_stage stage, shader_meta meta);
		const shader_meta& get_meta(shader_stage stage) const;
	private:
		std::array<std::string, static_cast<int>(shader_stage::_count)> source_data;
		std::array<shader_meta, static_cast<int>(shader_stage::_count)> meta_data;
	};
}

#endif // TOPAZ_GL2_API_SHADER_HPP
