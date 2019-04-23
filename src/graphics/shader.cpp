#include "graphics/shader.hpp"
#include "utility/file.hpp"
#include "mesh.hpp"

#ifdef TOPAZ_OPENGL
namespace tz::platform
{
	OGLShader::OGLShader(const std::string& vertex_source, const std::string& tessellation_control_source, const std::string& tessellation_evaluation_source, const std::string& geometry_source, const std::string& fragment_source, bool compile, bool link, bool validate): OGLShaderProgram()
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
			this->setup_standard_attributes();
			this->link().report_if_fail(std::cout);
		}
		if(compile && link && validate)
			this->validate().report_if_fail(std::cout);
	}

	OGLShader::OGLShader(std::string path, bool compile, bool link, bool validate): OGLShader(::tz::utility::file::read(path + ".vertex.glsl"), ::tz::utility::file::read(path + ".tessellation_control.glsl"), ::tz::utility::file::read(path + ".tessellation_evaluation.glsl"), ::tz::utility::file::read(path + ".geometry.glsl"), ::tz::utility::file::read(path + ".fragment.glsl"), compile, link, validate) {}

	void OGLShader::setup_standard_attributes() const
	{
		using namespace tz::platform;
		this->bind_attribute_location(static_cast<GLuint>(StandardAttribute::POSITION), mesh_attribute::position_attribute);
		this->bind_attribute_location(static_cast<GLuint>(StandardAttribute::TEXCOORD), mesh_attribute::texcoord_attribute);
		this->bind_attribute_location(static_cast<GLuint>(StandardAttribute::NORMAL), mesh_attribute::normal_attribute);
		this->bind_attribute_location(static_cast<GLuint>(StandardAttribute::TANGENT), mesh_attribute::tangent_attribute);
		this->bind_attribute_location(static_cast<GLuint>(StandardAttribute::INSTANCE_MODEL_X_ROW), mesh_attribute::instance_model_x_attribute);
		this->bind_attribute_location(static_cast<GLuint>(StandardAttribute::INSTANCE_MODEL_Y_ROW), mesh_attribute::instance_model_y_attribute);
		this->bind_attribute_location(static_cast<GLuint>(StandardAttribute::INSTANCE_MODEL_Z_ROW), mesh_attribute::instance_model_z_attribute);
		this->bind_attribute_location(static_cast<GLuint>(StandardAttribute::INSTANCE_MODEL_W_ROW), mesh_attribute::instance_model_w_attribute);
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