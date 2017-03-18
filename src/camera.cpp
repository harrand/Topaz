#include "camera.hpp"

Camera::Camera(Vector3F pos, Vector3F rot)
{
	this->pos = pos;
	this->rot = rot;
}

Vector3F Camera::getPos() const
{
	return this->pos;
}

Vector3F Camera::getRot() const
{
	return this->rot;
}

Vector3F& Camera::getPosR()
{
	return this->pos;
}

Vector3F& Camera::getRotR()
{
	return this->rot;
}

Vector3F Camera::getForward() const
{
	Vector4F forward4 = this->getCameraMatrix() * Vector4F(0, 0, -1, 0);
	return Vector3F(forward4.getX(), forward4.getY(), forward4.getZ()).normalised();
}

Vector3F Camera::getBackward() const
{
	Vector4F backward4 = this->getCameraMatrix() * Vector4F(0, 0, 1, 0);
	return Vector3F(backward4.getX(), backward4.getY(), backward4.getZ()).normalised();
}

Vector3F Camera::getUp() const
{
	Vector4F up4 = this->getCameraMatrix() * Vector4F(0, 1, 0, 0);
	return Vector3F(up4.getX(), up4.getY(), up4.getZ()).normalised();
}

Vector3F Camera::getDown() const
{
	Vector4F down4 = this->getCameraMatrix() * Vector4F(0, -1, 0, 0);
	return Vector3F(down4.getX(), down4.getY(), down4.getZ()).normalised();
}

Vector3F Camera::getLeft() const
{
	Vector4F left4 = this->getCameraMatrix() * Vector4F(-1, 0, 0, 0);
	return Vector3F(left4.getX(), left4.getY(), left4.getZ()).normalised();
}

Vector3F Camera::getRight() const
{
	Vector4F right4 = this->getCameraMatrix() * Vector4F(1, 0, 0, 0);
	return Vector3F(right4.getX(), right4.getY(), right4.getZ()).normalised();
}

Matrix4x4 Camera::getCameraMatrix() const
{
	return MatrixTransformations::createModelMatrix(this->pos, this->rot, Vector3F(1, 1, 1));
}