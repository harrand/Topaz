#ifndef QUATERNION_HPP
#define QUATERNION_HPP
#include "data/matrix.hpp"

/**
 * Specialised 4-dimensional Vector to represent rotations in 3-dimensional space.
 */
class Quaternion : protected Vector4F
{
public:
	/**
	 * Construct Quaternion from a rotation axis and an angle about the axis.
	 * @param rotation_axis - 3-dimensional Vector representing the rotation-axis, in euler-angles
	 * @param angle - The rotation angle, in radians
	 */
	Quaternion(Vector3F rotation_axis = Vector3F(), float angle = 0.0f);
	/**
	 * Construct Quaternion from three rotations in euler angles about the three spatial dimensions.
	 * @param euler_rotation - 3-dimensional Vector representing the angle of rotation in each axis, in radians
	 */
	Quaternion(Vector3F euler_rotation);
	/**
	 * Construct Quaternion from an existing rotational-matrix.
	 * @param rotational_matrix - The existing rotational-matrix
	 */
	Quaternion(Matrix4x4 rotational_matrix);

	/**
	 * Get the rotation angle.
	 * @return - Rotation angle, in radians
	 */
	float get_angle() const;
	/**
	 * Get the axis that the rotation is taking place in.
	 * @return - 3-dimensional Vector representing the rotation axis, in euler angles.
	 */
	Vector3F get_rotation_axis() const;
	using Vector4F::length;
	/**
	 * Convert the Quaternion to a rotatonal matrix
	 * @return - Row-major rotational matrix
	 */
	Matrix4x4 to_matrix() const;
	/**
	 * Get a normalised copy of this Quaternion.
	 * @return - Copy of this Quaternion, but normalised
	 */
	Quaternion normalised() const;
	/**
	 * Get a conjugate of the normalised Quaternion.
	 * @return - Copy of this Quaternion, but inversed
	 */
	Quaternion inverse() const;
	/**
	 * Unary operator- is the same as getting the conjugate of the Quaternion (reversing the polarity AKA getting opposite direction of the original rotation axis)
	 * @return - Copy of this Quaternion, but as a conjugate
	 */
	Quaternion operator-() const;
	/**
	 * Quaternion functor returns the Matrix (via Quaternion::to_matrix())
	 * @return - 4-dimensional rotational Matrix, equivalent to this Quaternion
	 */
	Matrix4x4 operator()() const;
	/**
	 * Enable implicit/explicit conversion to the Matrix4x4 component (via Quaternion::to_matrix())
	 * @return - 4-dimensional rotation Matrix, equal to this Quaternion
	 */
	operator Matrix4x4() const;
	/**
	 * Combine this Quaternion transformation with another.
	 * @param rhs - The other Quaternion to combine with this transformation
	 * @return - Combination of rotations of this Quaternion and the other
	 */
	Quaternion operator*(const Quaternion& rhs) const;
	/**
	 * Multiply the magnitude of this Quaternion by a scalar.
	 * @param scalar - Scalar to multiply by
	 * @return - Copy of this, but all attributes multiplied by the scalar
	 */
    Quaternion operator*(float scalar) const;
    /**
	 * Divide the magnitude of this Quaternion by a scalar.
	 * @param scalar - Scalar to divide by
	 * @return - Copy of this, but all attributes divide by the scalar
	 */
    Quaternion operator/(float scalar) const;
	/**
	 * Rotate a vector by a quaternion. Does not work for homogeneous coordinates; convert to a rotational matrix if you need to do that.
	 * @param vector - 3-dimensional Vector to be rotated by this Quaternion
	 * @return - The 3-dimensional Vector parameter, but rotated by this Quaternion
	 */
	Vector4F operator*(const Vector3F& vector) const;
private:
    /**
     * Construct a Quaternion directly from x, y, z and w components.
     * @param xyzw - Direct components
     */
	Quaternion(Vector4F xyzw);
};

#endif