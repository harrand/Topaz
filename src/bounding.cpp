#include "bounding.hpp"

AABB::AABB(Vector3F x, Vector3F y): min(x < y ? x : y), max(x > y ? x : y){}

bool AABB::intersects(const Vector3F& point) const
{
	return point < this->max && point > this->min;
}

const Vector3F& AABB::getMinimum() const
{
	return this->min;
}

const Vector3F& AABB::getMaximum() const
{
	return this->max;
}

Vector3F& AABB::getMinimumR()
{
	return this->min;
}

Vector3F& AABB::getMaximumR()
{
	return this->max;
}