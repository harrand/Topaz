#ifndef PHYSICS_HPP
#define PHYSICS_HPP
#include "physics/boundary.hpp"
#include "graphics/object.hpp"

/**
* Represent a physical force in three-dimensions.
*/
class Force
{
public:
	Force(Vector3F size = Vector3F());
	Force(const Force& copy) = default;
	Force(Force&& move) = default;
	~Force() = default;
	Force& operator=(const Force& rhs) = default;
	
	Force operator+(const Force& other) const;
	Force operator-(const Force& other) const;
	Force operator*(float rhs) const;
	Force operator/(float rhs) const;
	Force& operator+=(const Force& other);
	Force& operator-=(const Force& other);
	bool operator==(const Force& other) const;
	
	Vector3F size;
};

namespace tz::physics
{
	/**
	* Given an Object3D, creates a well-fitted sphere-boundary around the vertices of the object.
	* The BoundingSphere remains valid for the entire lifetime of the object.
	*/
	BoundingSphere bound_sphere(const Object& object);
	/**
	* Given an Object3D, creates a well-fitted axis-aligned-bounding-box around the vertices of the object.
	* The AABB remains valid for the entire lifetime of the object.
	*/
	AABB bound_aabb(const Object& object);
}

#endif