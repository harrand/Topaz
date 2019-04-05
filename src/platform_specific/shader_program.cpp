//
// Created by Harrand on 05/04/2019.
//

#include "shader_program.hpp"

#ifdef TOPAZ_OPENGL
namespace tz::platform
{
    OGLShaderComponentCompileResult::OGLShaderComponentCompileResult(const OGLShaderComponent& component): success(false), error_message(std::nullopt)
    {
        GLint success = 0;
        glGetShaderiv(component.shader_handle, GL_COMPILE_STATUS, &success);
        if(success == GL_TRUE)
        {
            this->success = true;
            std::string error_message_buffer;
            error_message_buffer.resize(4096);
            GLsizei actual_length;
            glGetShaderInfoLog(component.shader_handle, static_cast<GLsizei>(error_message_buffer.size()), &actual_length, error_message_buffer.data());
            error_message_buffer.resize(static_cast<std::size_t>(actual_length));
            this->error_message = std::move(error_message_buffer);
        }
        else
            this->success = false;
    }

    bool OGLShaderComponentCompileResult::was_successful() const
    {
        return this->success;
    }

    const std::string* OGLShaderComponentCompileResult::get_error_message() const
    {
        if(this->error_message.has_value())
            return &this->error_message.value();
        else
            return nullptr;
    }

    OGLShaderComponent::OGLShaderComponent(tz::platform::OGLShaderComponentType type, const std::optional<std::string>& source): shader_handle(0), type(type)
    {
        this->shader_handle = glCreateShader(static_cast<GLenum>(this->type));
        // If source is passed into constructor, upload and compile it instantly.
        if(source.has_value())
        {
            this->upload_source(source.value());
            this->compile();
        }
    }

    const OGLShaderComponentType& OGLShaderComponent::get_type() const
    {
        return this->type;
    }

    void OGLShaderComponent::upload_source(const std::string &source) const
    {
        auto length = static_cast<GLint>(source.length());
        const GLchar* source_data = source.c_str();
        glShaderSource(this->shader_handle, 1, &source_data, &length);
    }

    OGLShaderComponentCompileResult OGLShaderComponent::compile() const
    {
        glCompileShader(this->shader_handle);
        return this->get_compile_result();
    }

    OGLShaderComponentCompileResult OGLShaderComponent::get_compile_result() const
    {
        return {*this};
    }

    OGLShaderProgramLinkResult::OGLShaderProgramLinkResult(const OGLShaderProgram& shader_program): was_attempted(true), success(false), error_message(std::nullopt)
    {
        GLint success = 0;
        glGetProgramiv(shader_program.program_handle, GL_LINK_STATUS, &success);
        if(success == GL_TRUE)
        {
            this->success = true;
            std::string error_message_buffer;
            error_message_buffer.resize(4096);
            GLsizei actual_length;
            glGetProgramInfoLog(shader_program.program_handle, static_cast<GLsizei>(error_message_buffer.size()), &actual_length, error_message_buffer.data());
            error_message_buffer.resize(static_cast<std::size_t>(actual_length));
            this->error_message = std::move(error_message_buffer);
        }
        else
            this->success = false;
    }

    OGLShaderProgramLinkResult::OGLShaderProgramLinkResult(): was_attempted(false), success(false), error_message("Not all components were previously compiled"){}

    bool OGLShaderProgramLinkResult::get_was_attempted() const
    {
        return this->was_attempted;
    }

    bool OGLShaderProgramLinkResult::was_successful() const
    {
        return this->success;
    }

    OGLShaderProgramValidateResult::OGLShaderProgramValidateResult(const OGLShaderProgram &shader_program): was_attempted(true), success(false), error_message(std::nullopt)
    {
        GLint success = 0;
        glGetProgramiv(shader_program.program_handle, GL_VALIDATE_STATUS, &success);
        if(success == GL_TRUE)
        {
            this->success = true;
            std::string error_message_buffer;
            error_message_buffer.resize(4096);
            GLsizei actual_length;
            glGetProgramInfoLog(shader_program.program_handle, static_cast<GLsizei>(error_message_buffer.size()), &actual_length, error_message_buffer.data());
            error_message_buffer.resize(static_cast<std::size_t>(actual_length));
            this->error_message = std::move(error_message_buffer);
        }
        else
            this->success = false;
    }

    OGLShaderProgramValidateResult::OGLShaderProgramValidateResult(): was_attempted(false), success(false), error_message("The program has not been linked yet") {}

    OGLShaderProgram::OGLShaderProgram(): program_handle(glCreateProgram()), components(), uniforms(){}

    const OGLShaderComponent* OGLShaderProgram::get_component(OGLShaderComponentType type) const
    {
        for(auto& component_ptr : this->components)
            if(component_ptr->get_type() == type)
                return component_ptr.get();
        return nullptr;
    }

    bool OGLShaderProgram::is_fully_compiled() const
    {
        for(auto& component_ptr : this->components)
        {
            if(!component_ptr->get_compile_result().was_successful())
                return false;
        }
        return true;
    }

    OGLShaderProgramLinkResult OGLShaderProgram::link() const
    {
        if(!this->is_fully_compiled())
            return {};
        glLinkProgram(this->program_handle);
        return this->get_link_result();
    }

    OGLShaderProgramLinkResult OGLShaderProgram::get_link_result() const
    {
        if(!this->is_fully_compiled())
            return {};
        return {*this};
    }

    OGLShaderProgramValidateResult OGLShaderProgram::validate() const
    {
        if(!this->get_link_result().was_successful())
            return {};
        glValidateProgram(this->program_handle);
        return this->get_validate_result();
    }

    OGLShaderProgramValidateResult OGLShaderProgram::get_validate_result() const
    {
        if(!this->get_link_result().was_successful())
            return {};
        return {*this};
    }
}
#endif
