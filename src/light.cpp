#include "light.hpp"

BaseLight::BaseLight(const Vector3F& pos, const Vector3F& colour, const float power): pos(pos), colour(colour), power(power){}

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