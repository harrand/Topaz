#include "camera.hpp"

Camera::Camera(Vector3F pos, Vector3F rot, float fov, float near_clip, float far_clip): pos(std::move(pos)), rot(std::move(rot)), fov(fov), near_clip(near_clip), far_clip(far_clip){}

const Vector3F& Camera::getPosition() const
{
	return this->pos;
}

const Vector3F& Camera::getRotation() const
{
	return this->rot;
}

Vector3F& Camera::getPositionR()
{
	return this->pos;
}

Vector3F& Camera::getRotationR()
{
	return this->rot;
}

float Camera::getFOV() const
{
	return this->fov;
}

float Camera::getNearClip() const
{
	return this->near_clip;
}

float Camera::getFarClip() const
{
	return this->far_clip;
}

float& Camera::getFOVR()
{
	return this->fov;
}

float& Camera::getNearClipR()
{
	return this->near_clip;
}

float& Camera::getFarClipR()
{
	return this->far_clip;
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
	return Matrix4x4::createModelMatrix(this->pos, this->rot, Vector3F(1, 1, 1));
}