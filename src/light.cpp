#include "light.hpp"

BaseLight::BaseLight(Vector3F pos, Vector3F colour, float power): pos(std::move(pos)), colour(std::move(colour)), power(power){}

const Vector3F& BaseLight::getPosition() const
{
	return this->pos;
}

Vector3F& BaseLight::getPositionR()
{
	return this->pos;
}

const Vector3F& BaseLight::getColour() const
{
	return this->colour;
}

Vector3F& BaseLight::getColourR()
{
	return this->colour;
}

float BaseLight::getPower() const
{
	return this->power;
}