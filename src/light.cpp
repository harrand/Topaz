#include "light.hpp"

Light::Light(Vector3F pos, Vector3F colour, float power, float diffuse_component, float specular_component): pos(std::move(pos)), colour(std::move(colour)), power(power), diffuse_component(diffuse_component), specular_component(specular_component){}

const Vector3F& Light::get_position() const
{
	return this->pos;
}

const Vector3F& Light::get_colour() const
{
	return this->colour;
}

void Light::set_position(Vector3F position)
{
	this->pos = position;
}

void Light::set_colour(Vector3F colour)
{
	this->colour = colour;
}

float Light::get_power() const
{
	return this->power;
}

float Light::get_diffuse_component() const
{
	return this->diffuse_component;
}

float Light::get_specular_component() const
{
	return this->specular_component;
}

std::array<GLint, tz::graphics::light_number_of_uniforms> Light::get_uniforms(GLuint shader_program_handle, std::size_t light_uniform_index) const
{
	// nice cheap temporary std::array
return
	{	
	glGetUniformLocation(shader_program_handle, ("lights[" + tz::util::cast::to_string(light_uniform_index) + "].pos").c_str()),
	glGetUniformLocation(shader_program_handle, ("lights[" + tz::util::cast::to_string(light_uniform_index) + "].colour").c_str()),
	glGetUniformLocation(shader_program_handle, ("lights[" + tz::util::cast::to_string(light_uniform_index) + "].power").c_str()),
	glGetUniformLocation(shader_program_handle, ("lights[" + tz::util::cast::to_string(light_uniform_index) + "].diffuse_component").c_str()),
	glGetUniformLocation(shader_program_handle, ("lights[" + tz::util::cast::to_string(light_uniform_index) + "].specular_component").c_str())
	};
}