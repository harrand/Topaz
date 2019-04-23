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
	namespace util
	{
		/**
		* Get a label for a shader_type (e.g GL_FRAGMENT_SHADER returns "Fragment")
		*/
		const char* shader_type_string(GLenum shader_type);
	}
}
#endif