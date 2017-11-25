#include "light.hpp"

Light::Light(Vector3F position, Vector3F colour, float power, float diffuse_component, float specular_component): position(std::move(position)), colour(std::move(colour)), power(power), diffuse_component(diffuse_component), specular_component(specular_component){}

std::array<GLint, tz::graphics::light_number_of_uniforms> Light::get_uniforms(GLuint shader_program_handle, std::size_t light_uniform_index) const
{
	// nice cheap temporary std::array
return
	{	
	glGetUniformLocation(shader_program_handle, ("lights[" + tz::util::cast::to_string(light_uniform_index) + "].position").c_str()),
	glGetUniformLocation(shader_program_handle, ("lights[" + tz::util::cast::to_string(light_uniform_index) + "].colour").c_str()),
	glGetUniformLocation(shader_program_handle, ("lights[" + tz::util::cast::to_string(light_uniform_index) + "].power").c_str()),
	glGetUniformLocation(shader_program_handle, ("lights[" + tz::util::cast::to_string(light_uniform_index) + "].diffuse_component").c_str()),
	glGetUniformLocation(shader_program_handle, ("lights[" + tz::util::cast::to_string(light_uniform_index) + "].specular_component").c_str())
	};
}

bool Light::operator==(const Light& rhs) const
{
	return this->position == rhs.position && this->colour == rhs.colour && this->power == rhs.power && this->diffuse_component == rhs.diffuse_component && this->specular_component == rhs.specular_component;
}