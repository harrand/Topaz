#include "uniform.hpp"

#ifdef TOPAZ_DEBUG
namespace tz::platform
{
    UniformImplicit::UniformImplicit(GLuint shader_handle, std::string uniform_location): shader_handle(shader_handle), uniform_location(uniform_location) {}

    GLuint UniformImplicit::get_shader_handle() const
    {
        return this->shader_handle;
    }

    std::string_view UniformImplicit::get_uniform_location() const
    {
        return {this->uniform_location};
    }

    Uniform<DirectionalLight>::Uniform(GLuint shader_handle, std::string uniform_location, DirectionalLight value): UniformImplicit(shader_handle, uniform_location), value(value), direction_uniform_handle(glGetUniformLocation(this->shader_handle, (uniform_location + ".direction").c_str())),colour_uniform_handle(glGetUniformLocation(this->shader_handle, (uniform_location + ".colour").c_str())),power_uniform_handle(glGetUniformLocation(this->shader_handle, (uniform_location + ".power").c_str())){}

    const DirectionalLight &Uniform<DirectionalLight>::get_value() const
    {
        return this->value;
    }

    void Uniform<DirectionalLight>::set_value(DirectionalLight value)
    {
        this->value = value;
    }

    void Uniform<DirectionalLight>::push() const
    {
        glUniform3f(this->direction_uniform_handle, this->value.get_direction().x, this->value.get_direction().y, this->value.get_direction().z);
        glUniform3f(this->colour_uniform_handle, this->value.get_colour().x, this->value.get_colour().y, this->value.get_colour().z);
        glUniform1f(this->power_uniform_handle, this->value.get_power());
    }

    Uniform<PointLight>::Uniform(GLuint shader_handle, std::string uniform_location, PointLight value): UniformImplicit(shader_handle, uniform_location), value(value), position_uniform_handle(glGetUniformLocation(this->shader_handle, (uniform_location + ".position").c_str())),colour_uniform_handle(glGetUniformLocation(this->shader_handle, (uniform_location + ".colour").c_str())),power_uniform_handle(glGetUniformLocation(this->shader_handle, (uniform_location + ".power").c_str())){}

    const PointLight &Uniform<PointLight>::get_value() const
    {
        return this->value;
    }

    void Uniform<PointLight>::set_value(PointLight value)
    {
        this->value = value;
    }

    void Uniform<PointLight>::push() const
    {
        glUniform3f(this->position_uniform_handle, this->value.position.x, this->value.position.y, this->value.position.z);
        glUniform3f(this->colour_uniform_handle, this->value.get_colour().x, this->value.get_colour().y, this->value.get_colour().z);
        glUniform1f(this->power_uniform_handle, this->value.get_power());
    }
}
#endif