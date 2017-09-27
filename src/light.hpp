#ifndef LIGHT_HPP
#define LIGHT_HPP
#include "vector.hpp"
#include "graphics.hpp"

class Light
{
public:
	Light(Vector3F pos = Vector3F(), Vector3F colour = Vector3F(1, 1, 1), float power = 1.0f, float diffuse_component = 1.0f, float specular_component = 1.0f);
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
	std::vector<GLuint> getUniforms(GLuint shader_program_handle, std::size_t light_uniform_index) const;
private:
	Vector3F pos;
	Vector3F colour;
	float power;
	float diffuse_component;
	float specular_component;
};

#endif