#ifndef LIGHT_HPP
#define LIGHT_HPP
#include "vector.hpp"

class Light
{
public:
	Light(Vector3F pos = Vector3F(), Vector3F colour = Vector3F(1, 1, 1), float power = 1.0f);
	Light(const Light& copy) = default;
	Light(Light&& move) = default;
	~Light() = default;
	Light& operator=(const Light& rhs) = default;
	
	const Vector3F& getPosition() const;
	Vector3F& getPositionR();
	const Vector3F& getColour() const;
	Vector3F& getColourR();
	float getPower() const;
private:
	Vector3F pos;
	Vector3F colour;
	float power;
};

#endif