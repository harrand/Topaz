#include "light.hpp"

Light::Light(Vector3F pos, Vector3F colour, float power): pos(std::move(pos)), colour(std::move(colour)), power(power){}

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