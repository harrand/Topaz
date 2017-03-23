#include "quaternion.hpp"

Quaternion::Quaternion(Vector3F rotationAxis, float angleRadians)
{
	this->x = rotationAxis.getX() * sin(angleRadians / 2);
	this->y = rotationAxis.getY() * sin(angleRadians / 2);
	this->z = rotationAxis.getZ() * sin(angleRadians / 2);
	this->w = cos(angleRadians / 2);
}

Quaternion::Quaternion(Vector3F eulerRotation)
{
	Vector3F vx(1, 0, 0), vy(0, 1, 0), vz(0, 0, 1);
	Quaternion xr(vx, eulerRotation.getX());
	Quaternion yr(vy, eulerRotation.getY());
	Quaternion zr(vz, eulerRotation.getZ());
	Quaternion res((xr * yr) * zr);
	this->x = res.getX();
	this->y = res.getY();
	this->z = res.getZ();
	this->w = res.getW();
}

Quaternion::Quaternion(Vector4F quat)
{
	this->x = quat.getX();
	this->y = quat.getY();
	this->z = quat.getZ();
	this->w = quat.getW();
}

float& Quaternion::getXR()
{
	return this->x;
}

float& Quaternion::getYR()
{
	return this->y;
}

float& Quaternion::getZR()
{
	return this->z;
}

float& Quaternion::getWR()
{
	return this->w;
}

float Quaternion::getX() const
{
	return this->x;
}

float Quaternion::getY() const
{
	return this->y;
}

float Quaternion::getZ() const
{
	return this->z;
}

float Quaternion::getW() const
{
	return this->w;
}

float Quaternion::getAngleRadians() const
{
	return (2 * acos(this->w));
}

Vector3F Quaternion::getRotationAxis() const
{
	float sin_a = sin(this->getAngleRadians() / 2);
	return Vector3F(this->x / sin_a, this->y / sin_a, this->z / sin_a);
}

Matrix4x4 Quaternion::getRotationalMatrix() const
{
	//The algorithm used gives a column-major matrix, will transpose at the end to get row-major.
	Vector4F x(1 - (2 * (this->y * this->y) + (this->z * this->z)), 2 * ((this->x * this->y) - (this->z * this->w)), 2 * ((this->x * this->z) + (this->y * this->w)), 0.0f);
	Vector4F y(2 * ((this->x * this->y) + (this->z * this->w)), 1 - (2 * ((this->x * this->x) + (this->z * this->z))), 2 * ((this->y * this->z) - (this->x * this->w)), 0.0f);
	Vector4F z(2 * ((this->x * this->z) - (this->y * this->w)), 2 * ((this->y * this->z) + (this->x * this->w)), 1 - (2 * ((this->x * this->x) + (this->y * this->y))), 0.0f);
	Vector4F w(0.0f, 0.0f, 0.0f, 1.0f);
	Matrix4x4 col(x, y, z, w);
	return col.transposed();
}

float Quaternion::length() const
{
	Vector4F quat(this->x, this->y, this->z, this->w);
	return quat.length();
}

Quaternion Quaternion::normalised() const
{
	Vector4F quat(this->x, this->y, this->z, this->w);
	float length = quat.length();
	return Quaternion(Vector4F(quat.getX() / length, quat.getY() / length, quat.getZ() / length, quat.getW() / length));
}

Quaternion Quaternion::conjugate() const
{
	Vector3F ra = this->getRotationAxis();
	float ang = this->getAngleRadians();
	return Quaternion(Vector3F(-ra.getX(), -ra.getY(), -ra.getZ()), ang);
}

Quaternion Quaternion::inverse() const
{
	Quaternion quat = *this;
	return (quat.normalised()).conjugate();
}

Quaternion Quaternion::operator*(const Quaternion& other) const
{
	float w = (this->w * other.getW()) - (this->x * other.getX()) - (this->y * other.getY()) - (this->z * other.getZ());
	float x = (this->w * other.getX()) + (this->x * other.getW()) + (this->y * other.getZ()) - (this->z * other.getY());
	float y = (this->w * other.getY()) + (this->y * other.getW()) + (this->z * other.getX()) - (this->x * other.getZ());
	float z = (this->w * other.getZ()) + (this->z * other.getW()) + (this->x * other.getY()) - (this->y * other.getX());
	return Quaternion(Vector4F(x, y, z, w));
}

Vector4F Quaternion::operator*(const Vector4F& other) const
{
	return (this->getRotationalMatrix()) * other;
}

Matrix4x4 MatrixTransformations::createQuaternionSourcedModelMatrix(Vector3F position, Vector3F eulerRotation, Vector3F scale)
{
	return MatrixTransformations::createTranslationMatrix(position) * Quaternion(eulerRotation).getRotationalMatrix() * MatrixTransformations::createScalingMatrix(scale);
}

Matrix4x4 MatrixTransformations::createQuaternionSourcedViewMatrix(Vector3F cameraPosition, Vector3F cameraEulerRotation)
{
	return MatrixTransformations::createQuaternionSourcedModelMatrix(cameraPosition, cameraEulerRotation, Vector3F(1, 1, 1)).inverse();
}

Matrix4x4 MatrixTransformations::createMVPMatrix(Vector3F position, Vector3F eulerRotation, Vector3F scale, Vector3F cameraPosition, Vector3F cameraEulerRotation, float fov, float width, float height, float nearclip, float farclip)
{
	return MatrixTransformations::createProjectionMatrix(fov, width, height, nearclip, farclip) * MatrixTransformations::createViewMatrix(cameraPosition, cameraEulerRotation) * MatrixTransformations::createQuaternionSourcedModelMatrix(position, eulerRotation, scale);
}