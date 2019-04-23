#include "uniform.hpp"

#ifdef TOPAZ_OPENGL
namespace tz::platform
{
	UniformImplicit::UniformImplicit(const OGLShaderProgram* shader_program, std::string uniform_location): shader_program(shader_program), uniform_location(uniform_location) {}

	const std::string& UniformImplicit::get_uniform_location() const
	{
		return this->uniform_location;
	}

	Uniform<DirectionalLight>::Uniform(const OGLShaderProgram* shader_program, std::string uniform_location, DirectionalLight value): UniformImplicit(shader_program, uniform_location), value(value), direction_uniform_handle(this->shader_program->get_uniform_location(uniform_location + ".direction").value_or(-1)),colour_uniform_handle(this->shader_program->get_uniform_location(uniform_location + ".colour").value_or(-1)),power_uniform_handle(this->shader_program->get_uniform_location(uniform_location + ".power").value_or(-1)){}

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
		glUniform3f(this->colour_uniform_handle, this->value.get_colour().x, this->value.get_colour().y,
					this->value.get_colour().z);
		glUniform1f(this->power_uniform_handle, this->value.get_power());
	}

	Uniform<PointLight>::Uniform(const OGLShaderProgram* shader_program, std::string uniform_location, PointLight value): UniformImplicit(shader_program, uniform_location), value(value), position_uniform_handle(this->shader_program->get_uniform_location(uniform_location + ".position").value_or(-1)), colour_uniform_handle(this->shader_program->get_uniform_location(uniform_location + ".colour").value_or(-1)), power_uniform_handle(this->shader_program->get_uniform_location(uniform_location + ".power").value_or(-1)){}

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