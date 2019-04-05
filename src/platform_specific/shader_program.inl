#ifdef TOPAZ_OPENGL

namespace tz::platform
{
    template<typename... Args>
    OGLShaderComponent& OGLShaderProgram::emplace_shader_component(Args&&... args)
    {
        this->components.push_back(std::make_unique<OGLShaderComponent>(std::forward<Args>(args)...));
        return *this->components.back();
    }
}

#endif