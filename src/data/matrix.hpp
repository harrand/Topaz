#ifndef MATRIX_HPP
#define MATRIX_HPP
#include "data/vector.hpp"

/**
* Represents a two-dimensional square matrix.
*/
class Matrix2x2
{
public:
    /**
     * Construct a 2-dimensional Matrix row-by-row.
     * @param x - 2-dimensional Vector representing the first row
     * @param y - 2-dimensional Vector representing the second row
     */
	Matrix2x2(Vector2F x = Vector2F(1.0f, 0.0f), Vector2F y = Vector2F(0.0f, 1.0f));

	/**
	 * Find the determinant of the 2-dimensional Matrix.
	 * @return - Determinant of the Matrix
	 */
	float determinant() const;

	/// First row.
	Vector2F x;
	/// Second row.
	Vector2F y;
};


/**
* Represents a three-dimensional square matrix.
*/
class Matrix3x3
{
public:
    /**
     * Construct a 3-dimensional Matrix row-by-row.
     * @param x - 3-dimensional Vector representing the first row
     * @param y - 3-dimensional Vector representing the second row
     * @param z - 3-dimensional Vector representing the third row
     */
	Matrix3x3(Vector3F x = Vector3F(1.0f, 0.0f, 0.0f), Vector3F y = Vector3F(0.0f, 1.0f, 0.0f), Vector3F z = Vector3F(0.0f, 0.0f, 1.0f));

    /**
     * Find the determinant of the 3-dimensional Matrix.
     * @return - Determinant of the Matrix
     */
    float determinant() const;

    /// First row.
    Vector3F x;
    /// Second row.
    Vector3F y;
    /// Third row.
    Vector3F z;
};

/**
* Represents a four-dimensional square matrix.
*/
class Matrix4x4
{
public:
    /**
     * Construct a 4-dimensional Matrix row-by-row.
     * @param x - 4-dimensional Vector representing the first row
     * @param y - 4-dimensional Vector representing the second row
     * @param z - 4-dimensional Vector representing the third row
     * @param w - 4-dimensional Vector representing the fourth row
     */
	Matrix4x4(Vector4F x = Vector4F(1.0f, 0.0f, 0.0f, 0.0f), Vector4F y = Vector4F(0.0f, 1.0f, 0.0f, 0.0f), Vector4F z = Vector4F(0.0f, 0.0f, 1.0f, 0.0f), Vector4F w = Vector4F(0.0f, 0.0f, 0.0f, 1.0f));

	/**
	 * Create the 4-dimensional identity matrix:
	 * |1, 0, 0, 0| x
	 * |0, 1, 0, 0| y
	 * |0, 0, 1, 0| z
	 * |0, 0, 0, 1| w
	 * @return - Identity matrix
	 */
	static Matrix4x4 identity();
	/**
	 * Get a transposed copy of this Matrix.
	 * @return - Copy of this, but transposed
	 */
	Matrix4x4 transposed() const;
	/**
	 * Get the underlying data xyzw.
	 * @return xyzw, as an array of floats
	 */
	std::array<float, 16> fill_data() const;
	/**
	 * Create a 3-dimensional sub-matrix from a 2-dimensional point in the Matrix.
	 * @param iter_i - Position in the Matrix to begin the sub-matrix from
	 * @param iter_j - Position in the Matrix to end the sub-matrix with
	 * @return - The 3-dimensional sub-matrix
	 */
	Matrix3x3 sub_matrix(float iter_i, float iter_j) const;
	/**
	 * Transform an existing 4-dimensional Vector via this Matrix.
	 * @param other - The 4-dimensional Vector to transform
	 * @return - Copy of the specified Vector, after transformation
	 */
	Vector4F operator*(const Vector4F& other) const;
	/**
	 * Combine this transformation with another
	 * @param other - The transformation to combine with this
	 * @return - Combination of this and the other Matrix
	 */
	Matrix4x4 operator*(const Matrix4x4& other) const;
    /**
     * Find the determinant of the 4-dimensional Matrix.
     * @return - Determinant of the Matrix
     */
    float determinant() const;
    /**
     * Get the inverse of this Matrix.
     * @return - Copy of this, but inversed
     */
	Matrix4x4 inverse() const;

    /// First row.
    Vector4F x;
    /// Second row.
    Vector4F y;
    /// Third row.
    Vector4F z;
    /// Fourth row.
    Vector4F w;
};

/**
* Topaz transformation matrices are always in row-major format.
*/
namespace tz::transform
{
	/**
	 * Construct a four-dimensional row-major translation matrix.
	 * @param position - 3-dimensional Vector representing position to translate
	 * @return - The resultant translation matrix
	 */
	Matrix4x4 translate(const Vector3F& position);
	/**
	 * Construct a four-dimensional row-major rotational matrix in the x-axis (Pitch).
	 * @param euler_x - The pitch
	 * @return - The resultant rotational matrix
	 */
	Matrix4x4 rotate_x(float euler_x);
    /**
     * Construct a four-dimensional row-major rotational matrix in the y-axis (Yaw).
     * @param euler_y - The yaw
     * @return - The resultant rotational matrix
     */
	Matrix4x4 rotate_y(float euler_y);
    /**
     * Construct a four-dimensional row-major rotational matrix in the z-axis (Roll).
     * @param euler_z - The roll
     * @return - The resultant rotational matrix
     */
	Matrix4x4 rotate_z(float euler_z);
	/**
	 * Construct a four-dimensional row-major rotational matrix using XYZ rotations (Pitch, Yaw, Roll in euler-angles).
	 * @param euler_rotation - 3-dimensional Vector holding the pitch, yaw and roll in-order
	 * @return - The resultant rotational matrix
	 */
	Matrix4x4 rotate(const Vector3F& euler_rotation);
	Vector3F decompose_rotation(const Matrix4x4 rotational_matrix);
	/**
	 * Construct a four-dimensional row-major scaling matrix.
	 * @param scale - 3-dimensional Vector representing the scale factor in each spatial dimension
	 * @return - The resultant scaling matrix
	 */
	Matrix4x4 scale(const Vector3F& scale);
	Matrix4x4 look_at(const Vector3F& position, const Vector3F& target, const Vector3F& up = {0, 1, 0});
	/**
	 * Construct a row-major model matrix.
	 * @param position - 3-dimensional Vector representing position
	 * @param euler_rotation - 3-dimensional Vector representing rotation, in euler-angles
	 * @param scale - 3-dimensional Vector representing scale factors in each spatial dimension
	 * @return - The resultant model matrix
	 */
	Matrix4x4 model(const Vector3F& position, const Vector3F& euler_rotation, const Vector3F& scale);
	/**
	* Construct a row-major view matrix.
	* Works similarly to gluLookAt(...)
	 * @param camera_position - 3-dimensional Vector representing position of the camera-view, in world-space
	 * @param camera_euler_rotation - 3-dimensional Vector representing orientation of the camera, in euler-angles
	 * @return - The resultant view matrix
	 */
	Matrix4x4 view(const Vector3F& camera_position, const Vector3F& camera_euler_rotation);
	/**
	 * Construct a row-major projection Matrix via an orthographic projection.
	 * @param right - Right edge of the Matrix frustum
	 * @param left - Left edge of the Matrix frustum
	 * @param top - Top edge of the Matrix frustum
	 * @param bottom - Bottom edge of the Matrix frustum
	 * @param near - Near edge of the Matrix frustum
	 * @param far - Far edge of the Matrix frustum
	 * @return - The resultant projection matrix
	 */
	Matrix4x4 orthographic_projection(float right, float left, float top, float bottom, float near, float far);
	/**
	 * Construct a row-major projection Matrix via a perspective projection.
	 * @param fov - Field-of-view, in radians
	 * @param width - Width of the viewport. Unit must be proportional to pixels
	 * @param height - Height of the viewport. Unit must be proportional to pixels
	 * @param near_clip - Distance from the origin to the near-plane
	 * @param far_clip - Distance from the origin to the far-plane
	 * @return - The resultant projection matrix
	 */
	Matrix4x4 perspective_projection(float fov, float width, float height, float near_clip, float far_clip);
}

#endif