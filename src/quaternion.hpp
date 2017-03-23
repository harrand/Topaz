#ifndef QUATERNION_HPP
#define QUATERNION_HPP
#include "matrix.hpp"

class Quaternion
{
public:
	Quaternion(Vector3F rotationAxis, float angleRadians);
	Quaternion(Vector3F eulerRotation);
	Quaternion(Vector4F quat);
	
	float& getXR();
	float& getYR();
	float& getZR();
	float& getWR();
	
	float getX() const;
	float getY() const;
	float getZ() const;
	float getW() const;
	
	float getAngleRadians() const;
	Vector3F getRotationAxis() const;
	Matrix4x4 getRotationalMatrix() const;
	
	float length() const;
	Quaternion normalised() const;
	Quaternion conjugate() const;
	Quaternion inverse() const;
	
	Quaternion operator*(const Quaternion& other) const;
	Vector4F operator*(const Vector4F& other) const;
private:
	float x, y, z, w;
};

namespace MatrixTransformations // extends from matrix.hpp
{
	Matrix4x4 createQuaternionSourcedModelMatrix(Vector3F position, Vector3F eulerRotation, Vector3F scale);
	Matrix4x4 createQuaternionSourcedViewMatrix(Vector3F cameraPosition, Vector3F cameraEulerRotation);
	Matrix4x4 createMVPMatrix(Vector3F position, Vector3F eulerRotation, Vector3F scale, Vector3F cameraPosition, Vector3F cameraEulerRotation, float fov, float width, float height, float nearclip, float farclip);
}

#endif