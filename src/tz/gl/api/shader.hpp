#ifndef TOPAZ_GL2_API_SHADER_HPP
#define TOPAZ_GL2_API_SHADER_HPP
#include <string_view>

namespace tz::gl
{
	enum class shader_stage
	{
		vertex,
		fragment,
		compute,
		tessellation_control,
		tessellation_evaluation,

		Count
	};

	struct shader_meta
	{

	};

	template<typename T>
	concept shader_info_type = requires(T t, shader_stage stage, std::string_view sv, shader_meta meta)
	{
		{t.set_shader(stage, sv)} -> std::same_as<void>;
		{t.get_shader(stage)} -> std::same_as<std::string_view>;
		{t.has_shader(stage)} -> std::same_as<bool>;

		{t.set_meta(stage, meta)} -> std::same_as<void>;
		{t.get_meta(stage)} -> std::convertible_to<shader_meta>;
	};
}

#endif // TOPAZ_GL2_API_SHADER_HPP
