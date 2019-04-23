#ifndef SHADER_HPP
#define SHADER_HPP
#include "platform_specific/shader_program.hpp"

#ifdef TOPAZ_OPENGL
namespace tz::platform
{
	class OGLShader : public OGLShaderProgram
	{
	public:
		OGLShader(const std::string& vertex_source, const std::string& tessellation_control_source, const std::string& tessellation_evaluation_source, const std::string& geometry_source, const std::string& fragment_source, bool compile = true, bool link = true, bool validate = true);
		OGLShader(std::string path, bool compile = true, bool link = true, bool validate = true);
		void setup_standard_attributes() const;
	};
}
using Shader = tz::platform::OGLShader;
#endif

namespace tz
{
	namespace graphics
	{
		constexpr std::size_t texture_sampler_id = 0;
		constexpr std::size_t texture_cubemap_sampler_id = 0;
		constexpr std::size_t normal_map_sampler_id = 1;
		constexpr std::size_t parallax_map_sampler_id = 2;
		constexpr std::size_t displacement_map_sampler_id = 3;
		constexpr std::size_t specular_map_sampler_id = 4;
		constexpr std::size_t emissive_map_sampler_id = 5;
		// Allow a few more ids for extra assets which could appear in the future.
		constexpr std::size_t initial_extra_texture_sampler_id = 16;

		constexpr char render_shader_model_uniform_name[] = "m";
		constexpr char render_shader_view_uniform_name[] = "v";
		constexpr char render_shader_projection_uniform_name[] = "p";
		/**
		* Vertex, Tessellation Control, Tessellation Evaluation, Geometry, Fragment.
		*/
		constexpr std::size_t maximum_shaders = 5;
		constexpr std::size_t maximum_uniforms = GL_MAX_UNIFORM_LOCATIONS;
	}
	namespace util
	{
		/**
		* Get a label for a shader_type (e.g GL_FRAGMENT_SHADER returns "Fragment")
		*/
		const char* shader_type_string(GLenum shader_type);
	}
}
#endif