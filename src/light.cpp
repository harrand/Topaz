#include "light.hpp"

BaseLight::BaseLight(const Vector3F& pos, const float power): pos(pos), power(power){}

BaseLight::BaseLight(BaseLight&& toMove): pos(toMove.getPos()), power(toMove.getPower()){}

const Vector3F& BaseLight::getPos() const
{
	return this->pos;
}

const float BaseLight::getPower() const
{
	return this->power;
}