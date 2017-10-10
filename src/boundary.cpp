#include "boundary.hpp"
#include <algorithm>

BoundingSphere::BoundingSphere(Vector3F centre, float radius): Boundary(), centre(centre), radius(radius){}

const Vector3F& BoundingSphere::getCentre() const
{
	return this->centre;
}

float BoundingSphere::getRadius() const
{
	return this->radius;
}

bool BoundingSphere::intersects(const BoundingSphere& rhs) const
{
	/* returns true if the total radii is less than the distance between their centres
		** **
	   *  *  *
	    ** **
	*/
	float radius_distance = this->radius + rhs.getRadius();
	float centre_distance = (this->centre - rhs.getCentre()).length();
	return centre_distance < radius_distance;
}

bool BoundingSphere::intersects(Boundary* other_boundary) const
{
	if(dynamic_cast<BoundingSphere*>(other_boundary) == nullptr)
		return false;
	return this->intersects(*dynamic_cast<BoundingSphere*>(other_boundary));
}

AABB::AABB(Vector3F minimum, Vector3F maximum): Boundary(), minimum(minimum), maximum(maximum){}

const Vector3F& AABB::getMinimum() const
{
	return this->minimum;
}

const Vector3F& AABB::getMaximum() const
{
	return this->maximum;
}

bool AABB::intersects(const AABB& rhs) const
{
	/* Returns true if the maximum distance between the boxes in any of the three dimensions is less than zero (aka there is no distance)
		*---*
		|  *|--*
		*--|*  |
		   *---*
	*/
	Vector3F forward_distance = rhs.getMinimum() - this->maximum;
	Vector3F backward_distance = this->minimum - rhs.getMaximum();
	Vector3F distance = std::max(forward_distance, backward_distance);
	return std::max({distance.getX(), distance.getY(), distance.getZ()}) < 0;
}

bool AABB::intersects(const Vector3F& point) const
{
	/* Returns true if the point is greater than the minimum but less than the maximum in all three dimensions
		*---*
		| * |
		*---*
	*/
	bool meet_x = this->minimum.getX() <= point.getX() && this->maximum.getX() >= point.getX();
	bool meet_y = this->minimum.getY() <= point.getY() && this->maximum.getY() >= point.getY();
	bool meet_z = this->minimum.getZ() <= point.getZ() && this->maximum.getZ() >= point.getZ();
	return meet_x && meet_y && meet_z;
}

bool AABB::intersects(Boundary* other_boundary) const
{
	if(dynamic_cast<AABB*>(other_boundary) == nullptr)
		return false;
	return this->intersects(*dynamic_cast<AABB*>(other_boundary));
}

BoundingPlane::BoundingPlane(Vector3F normal, float distance): Boundary(), normal(normal), distance(distance){}

const Vector3F& BoundingPlane::getNormal() const
{
	return this->normal;
}

float BoundingPlane::getDistance() const
{
	return this->distance;
}

BoundingPlane BoundingPlane::normalised() const
{
	float length = this->normal.length();
	return {this->normal.normalised(), this->distance / length};
}

bool BoundingPlane::intersects(const BoundingSphere& rhs) const
{
/*	Returns true if |dot product of the plane distance + the sphere centre| is less than the radius of the sphere
//  	  **/
//  	 * / *
//        /**
	return (std::fabs(this->normal.dot(rhs.getCentre()) + this->distance) - rhs.getRadius()) < 0;
}

bool BoundingPlane::intersects(Boundary* other_boundary) const
{
	if(dynamic_cast<BoundingSphere*>(other_boundary) == nullptr)
		return false;
	return this->intersects(*dynamic_cast<BoundingSphere*>(other_boundary));
}