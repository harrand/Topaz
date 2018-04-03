#ifndef QUATERNION_HPP
#define QUATERNION_HPP
#include "matrix.hpp"

/**
 * Specialised 4-dimensional Vector to represent rotations in 3-dimensional space.
 */
class Quaternion : protected Vector4F
{
public:
	/**
	* Construct Quaternion from a rotation axis and an angle about the axis.
	*/
	Quaternion(Vector3F rotation_axis = Vector3F(), float angle = 0.0f);
	/**
	* Construct Quaternion from three rotations in euler angles about the three spatial dimensions.
	*/
	Quaternion(Vector3F euler_rotation);
	/**
	* Construct Quaternion from an existing rotational matrix.
	*/
	Quaternion(Matrix4x4 rotational_matrix);
	
	float get_angle() const;
	Vector3F get_rotation_axis() const;
	using Vector4F::length;
	/**
	* Convert the Quaternion to a row-major rotational matrix.
	*/
	Matrix4x4 to_matrix() const;
	Quaternion normalised() const;
	/**
	* Just the same as the conjugate of the normalised.
	*/
	Quaternion inverse() const;
	/**
	* Unary operator- is the same as getting the conjugate of the Quaternion (reversing the polarity AKA getting opposite direction of the original rotation axis)
	*/
	Quaternion operator-() const;
	/**
	* Quaternion functor returns the Matrix (via Quaternion::to_matrix())
	*/
	Matrix4x4 operator()() const;
	/**
	* Enable implicit/explicit conversion to the Matrix4x4 component (via Quaternion::to_matrix())
	*/
	operator Matrix4x4() const;
	/**
	* Combine quaternion rotations.
	*/
	Quaternion operator*(const Quaternion& rhs) const;
    Quaternion operator*(float scalar) const;
    Quaternion operator/(float scalar) const;
	/**
	* Rotate a vector by a quaternion. Does not work for homogeneous coordinates; convert to a rotational matrix if you need to do that.
	*/
	Vector4F operator*(const Vector3F& vector) const;
private:
	Quaternion(Vector4F xyzw);
};

#endif