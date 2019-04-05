#include "platform_specific/uniform.hpp"
#ifdef TOPAZ_OPENGL

namespace tz::platform
{
    template<typename... Args>
    OGLShaderComponent& OGLShaderProgram::emplace_shader_component(Args&&... args)
    {
        this->components.push_back(std::make_unique<OGLShaderComponent>(std::forward<Args>(args)...));
        OGLShaderComponent& component = *this->components.back();
        glAttachShader(this->program_handle, component.shader_handle);
        return component;
    }

    template<typename T, typename... Args>
    Uniform<T>& OGLShaderProgram::emplace_uniform(Args&&... args)
    {
        this->uniforms.push_back(std::make_unique<Uniform<T>>(std::forward<Args>(args)...));
        return dynamic_cast<Uniform<T>&>(*this->uniforms.back());
    }

    template<typename T>
    void OGLShaderProgram::set_uniform(const std::string& uniform_location, T value)
    {
        for(auto& uniform_ptr : this->uniforms)
            if(uniform_ptr->get_uniform_location() == uniform_location)
            {
                dynamic_cast<Uniform<T>*>(uniform_ptr.get())->set_value(value);
                return;
            }
        this->emplace_uniform<T>(this, uniform_location, value);
    }

    template<typename T>
    const T* OGLShaderProgram::get_uniform_value(const std::string& uniform_location) const
    {
        auto uniform = this->get_uniform(uniform_location);
        const Uniform<T>* specific_uniform = dynamic_cast<const Uniform<T>*>(uniform);
        if(specific_uniform != nullptr)
            return &specific_uniform->get_value();
        return nullptr;
    }
}

#endif