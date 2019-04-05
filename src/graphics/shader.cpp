#include "graphics/shader.hpp"
#include "utility/file.hpp"

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
            this->bind_attribute_location(0, "position");
            this->bind_attribute_location(1, "texcoord");
            this->bind_attribute_location(2, "normal");
            this->bind_attribute_location(3, "tangent");
            this->bind_attribute_location(4, "instancing_model_x");
            this->bind_attribute_location(5, "instancing_model_y");
            this->bind_attribute_location(6, "instancing_model_z");
            this->bind_attribute_location(7, "instancing_model_w");

            this->link().report(std::cout);
        }
        if(compile && link && validate)
            this->validate().report(std::cout);
    }

    OGLShader::OGLShader(std::string path, bool compile, bool link, bool validate): OGLShader(::tz::utility::file::read(path + ".vertex.glsl"), ::tz::utility::file::read(path + ".tessellation_control.glsl"), ::tz::utility::file::read(path + ".tessellation_evaluation.glsl"), ::tz::utility::file::read(path + ".geometry.glsl"), ::tz::utility::file::read(path + ".fragment.glsl"), compile, link, validate) {}
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