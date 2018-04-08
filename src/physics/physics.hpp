#ifndef PHYSICS_HPP
#define PHYSICS_HPP
#include "boundary.hpp"
#include "../graphics/object.hpp"

/**
* Represent a physical force in three-dimensions.
*/
class Force
{
public:
	/**
	 * Construct a Force with via a 3-dimensional Vector.
	 * @param size - Magnitude of the Force in each axis
	 */
	Force(Vector3F size = Vector3F());

	/**
	 * Add two Forces.
	 * @param other - The Force to add to this
	 * @return - This + other
	 */
	Force operator+(const Force& other) const;
	/**
	 * Subtract two Forces.
	 * @param other - The Force to subtract from this
	 * @return - This - other
	 */
	Force operator-(const Force& other) const;
	/**
	 * Multiply this Force by a scalar.
	 * @param rhs - The scalar to multiply this Force by
	 * @return - [x * rhs, y * rhs, z * rhs]
	 */
	Force operator*(float rhs) const;
	/**
	 * Divide this Force by a scalar.
	 * @param rhs - The scalar to divide this Force by
	 * @return - [x ÷ rhs, y ÷ rhs, z ÷ rhs]
	 */
	Force operator/(float rhs) const;
	/**
	 * Add another Force to this and assign this to the result.
	 * @param other - The Force to add to this
	 * @return - This, where This = This + other
	 */
	Force& operator+=(const Force& other);
	/**
	 * Subtract another Force from this and assign this to the result.
	 * @param other - The Force to subtract from this
	 * @return - This, where This = This - other
	 */
	Force& operator-=(const Force& other);
	/**
	 * Equate this Force with another.
	 * @param other - The Force to equate with this
	 * @return - True if this is equal to the parameter. False otherwise
	 */
	bool operator==(const Force& other) const;

	/// 3-dimensional representations of magnitudes of the Force in each axis.
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