#include "graphics/shader.hpp"
#include "utility/string.hpp"
#include "io/file.hpp"
#include "mesh.hpp"

#ifdef TOPAZ_OPENGL
namespace tz::gl
{
	OGLShader::OGLShader(const std::string& vertex_source, const std::string& tessellation_control_source, const std::string& tessellation_evaluation_source, const std::string& geometry_source, const std::string& fragment_source, bool compile, bool link, bool validate, ShaderAttributeCollection attribute_collection): OGLShaderProgram(), attribute_collection(attribute_collection)
	{
		if(compile)
		{
			this->emplace_shader_component(OGLShaderComponentType::VERTEX, vertex_source).get_compile_result().report_if_fail(std::cout);
			if (tessellation_control_source != "")
				this->emplace_shader_component(OGLShaderComponentType::TESSELLATION_CONTROL, tessellation_control_source).get_compile_result().report_if_fail(std::cout);
			if (tessellation_evaluation_source != "")
				this->emplace_shader_component(OGLShaderComponentType::TESSELLATION_EVALUATION, tessellation_evaluation_source).get_compile_result().report_if_fail(std::cout);
			if (geometry_source != "")
				this->emplace_shader_component(OGLShaderComponentType::GEOMETRY, geometry_source).get_compile_result().report_if_fail(std::cout);
			this->emplace_shader_component(OGLShaderComponentType::FRAGMENT, fragment_source).get_compile_result().report_if_fail(std::cout);
		}
		if(compile && link)
		{
			this->setup_attributes();
			this->link().report_if_fail(std::cout);
		}
		if(compile && link && validate)
			this->validate().report_if_fail(std::cout);
	}

	OGLShader::OGLShader(std::string path, bool compile, bool link, bool validate, ShaderAttributeCollection attribute_collection): OGLShader(this->include_headers(path, ::tz::utility::file::read(path + ".vertex.glsl")), this->include_headers(path, ::tz::utility::file::read(path + ".tessellation_control.glsl")), this->include_headers(path, ::tz::utility::file::read(path + ".tessellation_evaluation.glsl")), this->include_headers(path, ::tz::utility::file::read(path + ".geometry.glsl")), this->include_headers(path, ::tz::utility::file::read(path + ".fragment.glsl")), compile, link, validate, attribute_collection) {}

	void OGLShader::setup_attributes() const
	{
		using namespace tz::gl;
		using namespace tz::consts::graphics::mesh;
		for(GLuint i = 0; i < this->attribute_collection.get_size(); i++)
		{
			this->bind_attribute_location(i, this->attribute_collection[i]);
		}
	}

	std::string OGLShader::include_headers(const std::string& path, const std::string& source) const
	{
		std::vector<std::string> source_lines = tz::utility::string::split_string(source, '\n');
		std::string parsed_source;
		for(auto& line : source_lines)
		{
			//tz::debug::print(line, "\n");
			if(tz::utility::string::begins_with(line, "#include"))
			{
				// Need to handle include. Includes must be relative to the PROGRAM LOCATION.
				tz::debug::print("OGLShader::include_headers(string&): Detected header include:\n");
				std::string include_path = path.substr(0, path.find_last_of("/\\")) + "/" + tz::utility::string::substring(line, 10, line.size() - 2);
				tz::debug::print("\tInclude path (relative to directory containing this shader) = \"", include_path, "\"\n");
				std::string include_source = tz::utility::file::read(include_path);
				tz::debug::print("\tShader Source:\n", include_source);
				line = include_source;
			}
			parsed_source += line + "\n";
		}
		return parsed_source;
	}
}
#endif
const char* tz::util::shader_type_string(GLenum shader_type)
{
	const char *shader_type_str;
	switch (shader_type)
	{
		case GL_COMPUTE_SHADER:
			shader_type_str = "Compute";
			break;
		case GL_VERTEX_SHADER:
			shader_type_str = "Vertex";
			break;
		case GL_TESS_CONTROL_SHADER:
			shader_type_str = "Tessellation Control";
			break;
		case GL_TESS_EVALUATION_SHADER:
			shader_type_str = "Tessellation Evaluation";
			break;
		case GL_GEOMETRY_SHADER:
			shader_type_str = "Geometry";
			break;
		case GL_FRAGMENT_SHADER:
			shader_type_str = "Fragment";
			break;
		default:
			shader_type_str = "Unknown";
			break;
	}
	return shader_type_str;
}