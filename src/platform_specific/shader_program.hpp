//
// Created by Harrand on 05/04/2019.
//

#ifndef TOPAZ_SHADER_PROGRAM_HPP
#define TOPAZ_SHADER_PROGRAM_HPP

#include "platform_specific/uniform.hpp"

#ifdef TOPAZ_OPENGL
namespace tz::platform
{
    enum class OGLShaderComponentType : GLenum
    {
        VERTEX = GL_VERTEX_SHADER,
        TESSELLATION_CONTROL = GL_TESS_CONTROL_SHADER,
        TESSELLATION_EVALUATION = GL_TESS_EVALUATION_SHADER,
        GEOMETRY = GL_GEOMETRY_SHADER,
        FRAGMENT = GL_FRAGMENT_SHADER,
        COMPUTE = GL_COMPUTE_SHADER
    };

    class OGLShaderComponent;

    class OGLShaderComponentCompileResult
    {
    public:
        OGLShaderComponentCompileResult(const OGLShaderComponent& component);
        bool was_successful() const;
        const std::string* get_error_message() const;
    private:
        bool success;
        std::optional<std::string> error_message;
    };

    class OGLShaderComponent
    {
    public:
        OGLShaderComponent(OGLShaderComponentType type, const std::optional<std::string>& source = std::nullopt);
        const OGLShaderComponentType& get_type() const;
        void upload_source(const std::string& source) const;
        OGLShaderComponentCompileResult compile() const;
        OGLShaderComponentCompileResult get_compile_result() const;
        friend class OGLShaderComponentCompileResult;
    private:
        GLuint shader_handle;
        OGLShaderComponentType type;
    };

    class OGLShaderProgram;

    class OGLShaderProgramLinkResult
    {
    public:
        OGLShaderProgramLinkResult(const OGLShaderProgram& shader_program);
        OGLShaderProgramLinkResult();
        bool get_was_attempted() const;
        bool was_successful() const;
    private:
        bool was_attempted;
        bool success;
        std::optional<std::string> error_message;
    };

    class OGLShaderProgramValidateResult
    {
    public:
        OGLShaderProgramValidateResult(const OGLShaderProgram& shader_program);
        OGLShaderProgramValidateResult();
        bool get_was_attempted() const;
        bool was_successful() const;
    private:
        bool was_attempted;
        bool success;
        std::optional<std::string> error_message;
    };

    class OGLShaderProgram
    {
    public:
        OGLShaderProgram();
        const OGLShaderComponent* get_component(OGLShaderComponentType type) const;
        bool is_fully_compiled() const;
        OGLShaderProgramLinkResult link() const;
        OGLShaderProgramLinkResult get_link_result() const;
        OGLShaderProgramValidateResult validate() const;
        OGLShaderProgramValidateResult get_validate_result() const;
        template<typename... Args>
        OGLShaderComponent& emplace_shader_component(Args&&... args);

        friend class OGLShaderProgramLinkResult;
        friend class OGLShaderProgramValidateResult;
    private:
        GLuint program_handle;
        std::vector<std::unique_ptr<OGLShaderComponent>> components;
        std::vector<std::unique_ptr<UniformImplicit>> uniforms;
    };
}
#endif

#include "platform_specific/shader_program.inl"
#endif //TOPAZ_SHADER_PROGRAM_HPP
