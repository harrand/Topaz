#include "light.hpp"

BaseLight::BaseLight(const Vector3F& pos, const Vector3F& colour, const float power): pos(pos), colour(colour), power(power){}

const Vector3F& BaseLight::getPos() const
{
	return this->pos;
}

const Vector3F& BaseLight::getColour() const
{
	return this->colour;
}

float BaseLight::getPower() const
{
	return this->power;
}