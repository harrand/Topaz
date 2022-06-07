#ifndef TOPAZ_GL2_API_SHADER_HPP
#define TOPAZ_GL2_API_SHADER_HPP
#include <string_view>

namespace tz::gl
{
	enum class ShaderStage
	{
		Vertex,
		Fragment,
		Compute,
		TessellationControl,
		TessellationEvaluation,

		Count
	};

	struct ShaderMeta
	{

	};

	template<typename T>
	concept ShaderInfoType = requires(T t, ShaderStage stage, std::string_view sv, ShaderMeta meta)
	{
		{t.set_shader(stage, sv)} -> std::same_as<void>;
		{t.get_shader(stage)} -> std::same_as<std::string_view>;
		{t.has_shader(stage)} -> std::same_as<bool>;

		{t.set_meta(stage, meta)} -> std::same_as<void>;
		{t.get_meta(stage)} -> std::convertible_to<ShaderMeta>;
	};
}

#endif // TOPAZ_GL2_API_SHADER_HPP
