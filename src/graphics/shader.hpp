#ifndef SHADER_HPP
#define SHADER_HPP
#include "gl/shader_program.hpp"
#include "graphics/attribute_collection.hpp"

#ifdef TOPAZ_OPENGL
namespace tz::gl
{
	class OGLShader : public OGLShaderProgram
	{
	public:
		OGLShader(const std::string& vertex_source, const std::string& tessellation_control_source, const std::string& tessellation_evaluation_source, const std::string& geometry_source, const std::string& fragment_source, bool compile = true, bool link = true, bool validate = true, ShaderAttributeCollection attribute_collection = {});
		OGLShader(std::string path, bool compile = true, bool link = true, bool validate = true, ShaderAttributeCollection attribute_collection = {});
		void setup_attributes() const;
	private:
		std::string include_headers(const std::string& path, const std::string& source) const;

		ShaderAttributeCollection attribute_collection;
	};
}
using Shader = tz::gl::OGLShader;
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