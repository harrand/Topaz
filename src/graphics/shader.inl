#include "data/matrix.hpp"
#include "utility/log.hpp"

#ifdef TOPAZ_OPENGL
namespace tz::platform
{
    template<class T>
    void OGLShader::add_uniform(Uniform<T> &&uniform)
    {
        if(this->has_uniform(uniform.get_uniform_location()))
        {
            ::tz::debug::print("[OGLShader]: Tried to add uniform with location '", uniform.get_uniform_location(), "' to OGLShader with handle ", this->program_handle, ", of which the location is already occupied.\n");
            return;
        }
        if(this->uniform_counter >= ::tz::graphics::maximum_uniforms)
        {
            ::tz::debug::print("[OGLShader]: Tried to add uniform with location '", uniform.get_uniform_location(), "' to OGLShader with handle ", this->program_handle, ", but it already has the maximum number of uniforms attached (", ::tz::graphics::maximum_uniforms, ").\n");
            return;
        }
        this->uniform_data[uniform_counter++] = std::make_unique<Uniform<T>>(std::forward<Uniform<T>>(uniform));
    }

    template<class T>
    void OGLShader::emplace_uniform(std::string uniform_location, T value)
    {
        this->add_uniform<T>(Uniform<T>(this->program_handle, uniform_location, value));
    }

    template<class T>
    void OGLShader::set_uniform(std::string_view uniform_location, T value)
    {
        if(!this->has_uniform(uniform_location))
        {
            this->emplace_uniform(std::string(uniform_location), value);
            return;
        }
        for(std::size_t i = 0; i < this->uniform_counter; i++)
            if(this->uniform_data[i]->get_uniform_location() == uniform_location)
                dynamic_cast<Uniform<T> *>(this->uniform_data[i].get())->set_value(value);
    }

    template<class T>
    T OGLShader::get_uniform_value(std::string_view uniform_location) const
    {
        if(!this->has_uniform(uniform_location))
        {
            ::tz::debug::print("[OGLShader]: Tried to retrieve uniform_value for non-existent uniform. Returning default...\n");
            return T{};
        }
        return dynamic_cast<Uniform<T> *>(this->get_uniform(uniform_location))->get_value();
    }
}
#endif