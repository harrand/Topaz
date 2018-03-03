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
	*/
	AABB bound_aabb(const Object& object);
	/**
	 * Given multiple Objects, creates a single well-fitted axis-aligned-bounding-box as tightly as possible around all the objects.
	 * Perfect to be used on the result of a tz::graphics::batch(...) call to bound the entirety of the batch.
	 */
	AABB bound_aabb_batch(const std::vector<Object>& objects);
}

#endif