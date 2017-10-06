#ifndef LIGHT_HPP
#define LIGHT_HPP
#include "vector.hpp"
#include "graphics.hpp"

namespace tz
{
	namespace graphics
	{
		constexpr std::size_t light_number_of_uniforms = 5;
		constexpr Vector3F light_default_colour(std::array<float, 3>({1.0f, 1.0f, 1.0f}));
		constexpr float light_default_power = 1.0f;
		constexpr float light_default_diffuse_component = 1.0f;
		constexpr float light_default_specular_component = 10.0f;
	}
}

class Light
{
public:
	Light(Vector3F pos = Vector3F(), Vector3F colour = tz::graphics::light_default_colour, float power = tz::graphics::light_default_power, float diffuse_component = tz::graphics::light_default_diffuse_component, float specular_component = tz::graphics::light_default_specular_component);
	Light(const Light& copy) = default;
	Light(Light&& move) = default;
	~Light() = default;
	Light& operator=(const Light& rhs) = default;
	
	const Vector3F& getPosition() const;
	Vector3F& getPositionR();
	const Vector3F& getColour() const;
	Vector3F& getColourR();
	float getPower() const;
	float getDiffuseComponent() const;
	float getSpecularComponent() const;
	std::array<GLint, tz::graphics::light_number_of_uniforms> getUniforms(GLuint shader_program_handle, std::size_t light_uniform_index) const;
private:
	Vector3F pos;
	Vector3F colour;
	float power;
	float diffuse_component;
	float specular_component;
};

#endif