#include "light.hpp"

Light::Light(Vector3F pos, Vector3F colour, float power, float diffuse_component, float specular_component): pos(std::move(pos)), colour(std::move(colour)), power(power), diffuse_component(diffuse_component), specular_component(specular_component){}

const Vector3F& Light::getPosition() const
{
	return this->pos;
}

Vector3F& Light::getPositionR()
{
	return this->pos;
}

const Vector3F& Light::getColour() const
{
	return this->colour;
}

Vector3F& Light::getColourR()
{
	return this->colour;
}

float Light::getPower() const
{
	return this->power;
}

float Light::getDiffuseComponent() const
{
	return this->diffuse_component;
}

float Light::getSpecularComponent() const
{
	return this->specular_component;
}

std::vector<GLuint> Light::getUniforms(GLuint shader_program_handle, std::size_t light_uniform_index) const
{
	std::vector<GLuint> uniforms; // should really be an std::array<5>
	uniforms.push_back(glGetUniformLocation(shader_program_handle, ("lights[" + tz::util::cast::toString(light_uniform_index) + "].pos").c_str()));
	uniforms.push_back(glGetUniformLocation(shader_program_handle, ("lights[" + tz::util::cast::toString(light_uniform_index) + "].colour").c_str()));
	uniforms.push_back(glGetUniformLocation(shader_program_handle, ("lights[" + tz::util::cast::toString(light_uniform_index) + "].power").c_str()));
	uniforms.push_back(glGetUniformLocation(shader_program_handle, ("lights[" + tz::util::cast::toString(light_uniform_index) + "].diffuse_component").c_str()));
	uniforms.push_back(glGetUniformLocation(shader_program_handle, ("lights[" + tz::util::cast::toString(light_uniform_index) + "].specular_component").c_str()));
	return uniforms;
}