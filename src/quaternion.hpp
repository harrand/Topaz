#ifndef QUATERNION_HPP
#define QUATERNION_HPP
#include "matrix.hpp"

class Quaternion
{
public:
	Quaternion(Vector3F rotationAxis, float angleRadians);
	Quaternion(Vector3F eulerRotation);
	Quaternion(Vector4F quat);
	
	float& getX();
	float& getY();
	float& getZ();
	float& getW();
	
	float getAngleRadians() const;
	Vector3F getRotationAxis() const;
	Matrix4x4 getRotationalMatrix() const;
	
	float length() const;
	Quaternion normalised() const;
	Quaternion conjugate() const;
	Quaternion inverse() const;
	
	Quaternion operator*(Quaternion other);
	Vector4F operator*(Vector4F other);
private:
	float x, y, z, w;
};

namespace MatrixTransformations // extends from matrix.hpp
{
	Matrix4x4 createQuaternionSourcedModelMatrix(Vector3F position, Vector3F eulerRotation, Vector3F scale);
	Matrix4x4 createMVPMatrix(Vector3F position, Vector3F eulerRotation, Vector3F scale, Vector3F cameraPosition, Vector3F cameraEulerRotation, float fov, float width, float height, float nearclip, float farclip);
}

#endif