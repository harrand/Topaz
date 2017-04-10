#ifndef LIGHT_HPP
#define LIGHT_HPP
#include "vector.hpp"

class BaseLight
{
public:
	BaseLight(const Vector3F& pos = Vector3F(), const float power = 1.0f);
	BaseLight(BaseLight&& toMove);
	const Vector3F& getPos() const;
	const float getPower() const;
private:
	const Vector3F pos;
	const float power;
};

#endif