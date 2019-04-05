//
// Created by Harrand on 05/04/2019.
//

#ifndef TOPAZ_SHADER_PROGRAM_HPP
#define TOPAZ_SHADER_PROGRAM_HPP

#include "platform_specific/vertex_buffer.hpp"

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
        void report(std::ostream& str) const;
        void report_if_fail(std::ostream& str) const;
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
        friend class OGLShaderProgram;
    private:
        GLuint shader_handle;
        OGLShaderComponentType type;
    };

    using UniformLocation = GLint;
    class OGLShaderProgramLinkResult;
    class OGLShaderProgramValidateResult;
    class UniformImplicit;
    template<typename T>
    class Uniform;

    class OGLShaderProgram
    {
    public:
        OGLShaderProgram();
        bool get_can_tessellate() const;
        const OGLShaderComponent* get_shader_component(OGLShaderComponentType type) const;
        template<typename... Args>
        OGLShaderComponent& emplace_shader_component(Args&&... args);
        std::optional<UniformLocation> get_uniform_location(const std::string& uniform_name) const;
        template<typename T, typename... Args>
        Uniform<T>& emplace_uniform(Args&&... args);
        template<typename T>
        void set_uniform(const std::string& uniform_location, T value);
        const UniformImplicit* get_uniform(const std::string& uniform_location) const;
        template<typename T>
        const T* get_uniform_value(const std::string& uniform_location) const;
        bool has_uniform(const std::string& uniform_name) const;
        void bind_attribute_location(GLuint index, const std::string& name);
        bool is_fully_compiled() const;
        OGLShaderProgramLinkResult link() const;
        OGLShaderProgramLinkResult get_link_result() const;
        OGLShaderProgramValidateResult validate() const;
        OGLShaderProgramValidateResult get_validate_result() const;
        void bind() const;
        void update();

        friend class OGLShaderProgramLinkResult;
        friend class OGLShaderProgramValidateResult;
    private:
        GLuint program_handle;
        std::vector<std::unique_ptr<OGLShaderComponent>> components;
        std::vector<std::unique_ptr<UniformImplicit>> uniforms;
    };

    class OGLShaderProgramLinkResult
    {
    public:
        OGLShaderProgramLinkResult(const OGLShaderProgram& shader_program);
        OGLShaderProgramLinkResult();
        bool get_was_attempted() const;
        bool was_successful() const;
        void report(std::ostream& str) const;
        void report_if_fail(std::ostream& str) const;
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
        void report(std::ostream& str) const;
        void report_if_fail(std::ostream& str) const;
    private:
        bool was_attempted;
        bool success;
        std::optional<std::string> error_message;
    };
}
#endif
#include "platform_specific/shader_program.inl"

#endif //TOPAZ_SHADER_PROGRAM_HPP

