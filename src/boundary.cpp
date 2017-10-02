#include "boundary.hpp"
#include <algorithm>

BoundingSphere::BoundingSphere(Vector3F centre, float radius): centre(centre), radius(radius){}

const Vector3F& BoundingSphere::getCentre() const
{
	return this->centre;
}

Vector3F& BoundingSphere::getCentreR()
{
	return this->centre;
}

float BoundingSphere::getRadius() const
{
	return this->radius;
}

float& BoundingSphere::getRadiusR()
{
	return this->radius;
}

bool BoundingSphere::intersects(const BoundingSphere& rhs) const
{
	float radius_distance = this->radius + rhs.getRadius();
	float centre_distance = (this->centre - rhs.getCentre()).length();
	return centre_distance < radius_distance;
}

AABB::AABB(Vector3F minimum, Vector3F maximum): minimum(minimum), maximum(maximum){}

const Vector3F& AABB::getMinimum() const
{
	return this->minimum;
}

Vector3F& AABB::getMinimumR()
{
	return this->minimum;
}

const Vector3F& AABB::getMaximum() const
{
	return this->maximum;
}

Vector3F& AABB::getMaximumR()
{
	return this->maximum;
}

bool AABB::intersects(const AABB& rhs) const
{
	Vector3F forward_distance = rhs.getMinimum() - this->maximum;
	Vector3F backward_distance = this->minimum - rhs.getMaximum();
	Vector3F distance = std::max(forward_distance, backward_distance);
	return std::max({distance.getX(), distance.getY(), distance.getZ()}) < 0;
}