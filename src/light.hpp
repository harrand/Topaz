#ifndef LIGHT_HPP
#define LIGHT_HPP
#include "vector.hpp"

class BaseLight
{
public:
	BaseLight(const Vector3F& pos = Vector3F(), const Vector3F& colour = Vector3F(1, 1, 1), const float power = 1.0f);
	BaseLight(BaseLight&& toMove);
	const Vector3F& getPos() const;
	const Vector3F& getColour() const;
	const float getPower() const;
private:
	const Vector3F pos;
	const Vector3F colour;
	const float power;
};

#endif