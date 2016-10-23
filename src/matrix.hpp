#ifndef MATRIX_HPP
#define MATRIX_HPP
#include "vector.hpp"

class Matrix2x2
{
public:
	Matrix2x2(Vector2F x = Vector2F(1.0f, 0.0f), Vector2F y = Vector2F(0.0f, 1.0f));
	Vector2F getRowX();
	Vector2F getRowY();
	Vector2F& getRowXR();
	Vector2F& getRowYR();
	float determinant();
private:
	Vector2F x, y;
};

class Matrix3x3
{
public:
	Matrix3x3(Vector3F x = Vector3F(1.0f, 0.0f, 0.0f), Vector3F y = Vector3F(0.0f, 1.0f, 0.0f), Vector3F z = Vector3F(0.0f, 0.0f, 1.0f));
	Vector3F getRowX();
	Vector3F getRowY();
	Vector3F getRowZ();
	Vector3F& getRowXR();
	Vector3F& getRowYR();
	Vector3F& getRowZR();
	float determinant();
private:
	Vector3F x, y, z;
};

class Matrix4x4
{
public:
	Matrix4x4(Vector4F x = Vector4F(1.0f, 0.0f, 0.0f, 0.0f), Vector4F y = Vector4F(0.0f, 1.0f, 0.0f, 0.0f), Vector4F z = Vector4F(0.0f, 0.0f, 1.0f, 0.0f), Vector4F w = Vector4F(0.0f, 0.0f, 0.0f, 1.0f));
	static Matrix4x4 identity();
	Vector4F getRowX();
	Vector4F getRowY();
	Vector4F getRowZ();
	Vector4F getRowW();
	Vector4F& getRowXR();
	Vector4F& getRowYR();
	Vector4F& getRowZR();
	Vector4F& getRowWR();
	
	Matrix4x4 transposed();
	Matrix4x4 washed(float min = 0.005f, float max = 99999.0f);
	
	void fillData(float data[16]);
	Matrix3x3 subMatrix(float iterI, float iterJ);
	
	Matrix4x4 operator+(Matrix4x4 other);
	Matrix4x4 operator-(Matrix4x4 other);
	Vector4F operator*(Vector4F other);
	Matrix4x4 operator*(Matrix4x4 other);
	
	float determinant();
	Matrix4x4 inverse();
private:
	Vector4F x, y, z, w;
};

namespace MatrixTransformations
{
	Matrix4x4 createTranslationMatrix(Vector3F position);
	
	Matrix4x4 createRotationalXMatrix(float eulerX);
	Matrix4x4 createRotationalYMatrix(float eulerY);
	Matrix4x4 createRotationalZMatrix(float eulerZ);
	Matrix4x4 createRotationalMatrix(Vector3F eulerRotation);
	
	Matrix4x4 createScalingMatrix(Vector3F scale);
	
	Matrix4x4 createModelMatrix(Vector3F position, Vector3F eulerRotation, Vector3F scale);
	Matrix4x4 createViewMatrix(Vector3F cameraPosition, Vector3F cameraEulerRotation);
	Matrix4x4 createProjectionMatrix(float fov, float aspectRatio, float nearclip, float farclip);
	Matrix4x4 createProjectionMatrix(float fov, float width, float height, float nearclip, float farclip);
}

#endif