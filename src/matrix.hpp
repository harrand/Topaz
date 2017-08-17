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
	Matrix2x2& operator=(const Matrix2x2& rhs) = default;
	
	Vector2F getRowX() const;
	Vector2F getRowY() const;
	Vector2F& getRowXR();
	Vector2F& getRowYR();
	
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
	Matrix3x3& operator=(const Matrix3x3& rhs) = default;
	
	Vector3F getRowX() const;
	Vector3F getRowY() const;
	Vector3F getRowZ() const;
	Vector3F& getRowXR();
	Vector3F& getRowYR();
	Vector3F& getRowZR();
	
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
	Matrix4x4& operator=(const Matrix4x4& rhs) = default;
	
	static Matrix4x4 identity();
	
	Vector4F getRowX() const;
	Vector4F getRowY() const;
	Vector4F getRowZ() const;
	Vector4F getRowW() const;
	
	Vector4F& getRowXR();
	Vector4F& getRowYR();
	Vector4F& getRowZR();
	Vector4F& getRowWR();
	
	Matrix4x4 transposed() const;
	
	Matrix4x4 washed(float min = 0.005f, float max = 99999.0f) const;
	
	std::array<float, 16> fillData() const;
	
	Matrix3x3 subMatrix(float iterI, float iterJ) const;
	
	Matrix4x4 operator+(const Matrix4x4& other) const;
	Matrix4x4 operator-(const Matrix4x4& other) const;
	Vector4F operator*(const Vector4F& other) const;
	Matrix4x4 operator*(const Matrix4x4& other) const;
	
	float determinant() const;
	
	Matrix4x4 inverse() const;
	
	static Matrix4x4 createTranslationMatrix(Vector3F position);
	static Matrix4x4 createRotationalXMatrix(float eulerX);
	static Matrix4x4 createRotationalYMatrix(float eulerY);
	static Matrix4x4 createRotationalZMatrix(float eulerZ);
	static Matrix4x4 createRotationalMatrix(Vector3F euler_rotation);
	static Matrix4x4 createScalingMatrix(Vector3F scale);
	
	static Matrix4x4 createModelMatrix(Vector3F position, Vector3F euler_rotation, Vector3F scale);
	static Matrix4x4 createViewMatrix(Vector3F camera_position, Vector3F camera_euler_rotation);
	static Matrix4x4 createProjectionMatrix(float fov, float aspect_ratio, float nearclip, float farclip);
	static Matrix4x4 createProjectionMatrix(float fov, float width, float height, float nearclip, float farclip);
private:
	Vector4F x, y, z, w;
};

#endif