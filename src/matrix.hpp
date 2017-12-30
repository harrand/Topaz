#ifndef MATRIX_HPP
#define MATRIX_HPP
#include <array>
#include "vector.hpp"

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

namespace tz::transform
{
	Matrix4x4 translate(const Vector3F& position);
	Matrix4x4 rotate_x(float euler_x);
	Matrix4x4 rotate_y(float euler_y);
	Matrix4x4 rotate_z(float euler_z);
	Matrix4x4 rotate(const Vector3F& euler_rotation);
	Matrix4x4 scale(const Vector3F& scale);
	
	Matrix4x4 model(const Vector3F& position, const Vector3F& euler_rotation, const Vector3F& scale);
	Matrix4x4 view(const Vector3F& camera_position, const Vector3F& camera_euler_rotation);
	Matrix4x4 orthographic_projection(float right, float left, float top, float bottom, float near, float far);
	Matrix4x4 perspective_projection(float fov, float width, float height, float near_clip, float far_clip);
}

#endif