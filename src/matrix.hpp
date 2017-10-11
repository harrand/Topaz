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
	
	Vector2F get_row_x() const;
	Vector2F get_row_y() const;
	void set_row_x(Vector2F row_x);
	void set_row_y(Vector2F row_y);
	float determinant() const;
private:
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
	
	Vector3F get_row_x() const;
	Vector3F get_row_y() const;
	Vector3F get_row_z() const;
	void set_row_x(Vector3F row_x);
	void set_row_y(Vector3F row_y);
	void set_row_z(Vector3F row_z);
	float determinant() const;
private:
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
	Vector4F get_row_x() const;
	Vector4F get_row_y() const;
	Vector4F get_row_z() const;
	Vector4F get_row_w() const;
	void set_row_x(Vector4F row_x);
	void set_row_y(Vector4F row_y);
	void set_row_z(Vector4F row_z);
	void set_row_w(Vector4F row_w);
	Matrix4x4 transposed() const;
	std::array<float, 16> fill_data() const;
	Matrix3x3 sub_matrix(float iter_i, float iter_j) const;
	Matrix4x4 operator+(const Matrix4x4& other) const;
	Matrix4x4 operator-(const Matrix4x4& other) const;
	Vector4F operator*(const Vector4F& other) const;
	Matrix4x4 operator*(const Matrix4x4& other) const;
	float determinant() const;
	Matrix4x4 inverse() const;
	
	static Matrix4x4 create_translation_matrix(Vector3F position);
	static Matrix4x4 create_rotational_x_matrix(float euler_x);
	static Matrix4x4 create_rotational_y_matrix(float euler_y);
	static Matrix4x4 create_rotational_z_matrix(float euler_z);
	static Matrix4x4 create_rotational_matrix(Vector3F euler_rotation);
	static Matrix4x4 create_scaling_matrix(Vector3F scale);
	static Matrix4x4 create_model_matrix(Vector3F position, Vector3F euler_rotation, Vector3F scale);
	static Matrix4x4 create_view_matrix(Vector3F camera_position, Vector3F camera_euler_rotation);
	static Matrix4x4 create_orthographic_matrix(float right, float left, float top, float bottom, float near, float far);
	static Matrix4x4 create_perspective_matrix(float fov, float aspect_ratio, float nearclip, float farclip);
	static Matrix4x4 create_perspective_matrix(float fov, float width, float height, float nearclip, float farclip);
private:
	Vector4F x, y, z, w;
};

#endif