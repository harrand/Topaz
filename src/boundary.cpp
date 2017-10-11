#include "boundary.hpp"
#include <algorithm>

BoundingSphere::BoundingSphere(Vector3F centre, float radius): Boundary(), centre(centre), radius(radius){}

const Vector3F& BoundingSphere::get_centre() const
{
	return this->centre;
}

float BoundingSphere::get_radius() const
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
	float radius_distance = this->radius + rhs.get_radius();
	float centre_distance = (this->centre - rhs.get_centre()).length();
	return centre_distance < radius_distance;
}

bool BoundingSphere::intersects(Boundary* other_boundary) const
{
	if(dynamic_cast<BoundingSphere*>(other_boundary) == nullptr)
		return false;
	return this->intersects(*dynamic_cast<BoundingSphere*>(other_boundary));
}

AABB::AABB(Vector3F minimum, Vector3F maximum): Boundary(), minimum(minimum), maximum(maximum){}

const Vector3F& AABB::get_minimum() const
{
	return this->minimum;
}

const Vector3F& AABB::get_maximum() const
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
	Vector3F forward_distance = rhs.get_minimum() - this->maximum;
	Vector3F backward_distance = this->minimum - rhs.get_maximum();
	Vector3F distance = std::max(forward_distance, backward_distance);
	return std::max({distance.get_x(), distance.get_y(), distance.get_z()}) < 0;
}

bool AABB::intersects(const Vector3F& point) const
{
	/* Returns true if the point is greater than the minimum but less than the maximum in all three dimensions
		*---*
		| * |
		*---*
	*/
	bool meet_x = this->minimum.get_x() <= point.get_x() && this->maximum.get_x() >= point.get_x();
	bool meet_y = this->minimum.get_y() <= point.get_y() && this->maximum.get_y() >= point.get_y();
	bool meet_z = this->minimum.get_z() <= point.get_z() && this->maximum.get_z() >= point.get_z();
	return meet_x && meet_y && meet_z;
}

bool AABB::intersects(Boundary* other_boundary) const
{
	if(dynamic_cast<AABB*>(other_boundary) == nullptr)
		return false;
	return this->intersects(*dynamic_cast<AABB*>(other_boundary));
}

BoundingPlane::BoundingPlane(Vector3F normal, float distance): Boundary(), normal(normal), distance(distance){}

const Vector3F& BoundingPlane::get_normal() const
{
	return this->normal;
}

float BoundingPlane::get_distance() const
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
	return (std::fabs(this->normal.dot(rhs.get_centre()) + this->distance) - rhs.get_radius()) < 0;
}

bool BoundingPlane::intersects(Boundary* other_boundary) const
{
	if(dynamic_cast<BoundingSphere*>(other_boundary) == nullptr)
		return false;
	return this->intersects(*dynamic_cast<BoundingSphere*>(other_boundary));
}