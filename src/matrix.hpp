#ifndef MATRIX_HPP
#define MATRIX_HPP
#include <array>
#include "vector.hpp"

/**
* Represents a two-dimensional square matrix.
*/
class Matrix2x2
{
public:
	Matrix2x2(Vector2F x = Vector2F(1.0f, 0.0f), Vector2F y = Vector2F(0.0f, 1.0f));
	Matrix2x2(const Matrix2x2& copy) = default;
	Matrix2x2(Matrix2x2&& move) = default;
	~Matrix2x2() = default;
	Matrix2x2& operator=(const Matrix2x2& rhs) = default;
	
	float determinant() const;

	Vector2F x, y;
};


/**
* Represents a three-dimensional square matrix.
*/
class Matrix3x3
{
public:
	Matrix3x3(Vector3F x = Vector3F(1.0f, 0.0f, 0.0f), Vector3F y = Vector3F(0.0f, 1.0f, 0.0f), Vector3F z = Vector3F(0.0f, 0.0f, 1.0f));
	Matrix3x3(const Matrix3x3& copy) = default;
	Matrix3x3(Matrix3x3&& move) = default;
	~Matrix3x3() = default;
	Matrix3x3& operator=(const Matrix3x3& rhs) = default;
	
	float determinant() const;
	
	Vector3F x, y, z;
};

/**
* Represents a four-dimensional square matrix.
*/
class Matrix4x4
{
public:
	Matrix4x4(Vector4F x = Vector4F(1.0f, 0.0f, 0.0f, 0.0f), Vector4F y = Vector4F(0.0f, 1.0f, 0.0f, 0.0f), Vector4F z = Vector4F(0.0f, 0.0f, 1.0f, 0.0f), Vector4F w = Vector4F(0.0f, 0.0f, 0.0f, 1.0f));
	Matrix4x4(const Matrix4x4& copy) = default;
	Matrix4x4(Matrix4x4&& move) = default;
	~Matrix4x4() = default;
	Matrix4x4& operator=(const Matrix4x4& rhs) = default;
	
	static Matrix4x4 identity();
	Matrix4x4 transposed() const;
	std::array<float, 16> fill_data() const;
	Matrix3x3 sub_matrix(float iter_i, float iter_j) const;
	Vector4F operator*(const Vector4F& other) const;
	Matrix4x4 operator*(const Matrix4x4& other) const;
	float determinant() const;
	Matrix4x4 inverse() const;

	Vector4F x, y, z, w;
};

/**
* Topaz transformation matrices are always in row-major format.
*/
namespace tz::transform
{
	/**
	* Construct a four-dimensional row-major translation matrix.
	*/
	Matrix4x4 translate(const Vector3F& position);
	/**
	* Construct a four-dimensional row-major rotational matrix in the x-axis (Pitch).
	*/
	Matrix4x4 rotate_x(float euler_x);
	/**
	* Construct a four-dimensional row-major rotational matrix in the y-axis (Yaw).
	*/
	Matrix4x4 rotate_y(float euler_y);
	/**
	* Construct a four-dimensional row-major rotational matrix in the x-axis (Roll).
	*/
	Matrix4x4 rotate_z(float euler_z);
	/**
	* Construct a four-dimensional row-major rotational matrix using XYZ rotations (Pitch, Yaw, Roll in Euler-angles).
	*/
	Matrix4x4 rotate(const Vector3F& euler_rotation);
	/**
	* Construct a four-dimensional row-major scaling matrix.
	*/
	Matrix4x4 scale(const Vector3F& scale);
	
	/**
	* Construct a row-major model matrix using the functions above.
	*/
	Matrix4x4 model(const Vector3F& position, const Vector3F& euler_rotation, const Vector3F& scale);
	/**
	* Construct a row-major view matrix using the functions above.
	* Works similarly to gluLookAt(...)
	*/
	Matrix4x4 view(const Vector3F& camera_position, const Vector3F& camera_euler_rotation);
	/**
	* Construct a row-major projection matrix to create an orthographic projection.
	*/
	Matrix4x4 orthographic_projection(float right, float left, float top, float bottom, float near, float far);
	/**
	* Construct a row-major projection matrix to create a perspective projection.
	* Use this to simulate 3D with a "camera".
	*/
	Matrix4x4 perspective_projection(float fov, float width, float height, float near_clip, float far_clip);
}

#endif