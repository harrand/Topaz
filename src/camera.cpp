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

void Camera::setPosition(Vector3F position)
{
	this->pos = position;
}

void Camera::setRotation(Vector3F rotation)
{
	this->rot = rotation;
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

void Camera::setFOV(float fov)
{
	this->fov = fov;
}

void Camera::setNearClip(float near_clip)
{
	this->near_clip = near_clip;
}

void Camera::setFarClip(float far_clip)
{
	this->far_clip = far_clip;
}

Vector3F Camera::getForward() const
{
	// put the axis-aligned forward vector into world space with swizzling to cull the w component
	Vector4F forward4 = this->getCameraMatrix() * Vector4F(0, 0, -1, 0);
	return Vector3F(forward4.getX(), forward4.getY(), forward4.getZ()).normalised();
}

Vector3F Camera::getBackward() const
{
	// put the axis-aligned backward vector into world space with swizzling to cull the w component
	Vector4F backward4 = this->getCameraMatrix() * Vector4F(0, 0, 1, 0);
	return Vector3F(backward4.getX(), backward4.getY(), backward4.getZ()).normalised();
}

Vector3F Camera::getUp() const
{
	// put the axis-aligned upward vector into world space with swizzling to cull the w component
	Vector4F up4 = this->getCameraMatrix() * Vector4F(0, 1, 0, 0);
	return Vector3F(up4.getX(), up4.getY(), up4.getZ()).normalised();
}

Vector3F Camera::getDown() const
{
	// put the axis-aligned downward vector into world space with swizzling to cull the w component
	Vector4F down4 = this->getCameraMatrix() * Vector4F(0, -1, 0, 0);
	return Vector3F(down4.getX(), down4.getY(), down4.getZ()).normalised();
}

Vector3F Camera::getLeft() const
{
	// put the axis-aligned left vector into world space with swizzling to cull the w component
	Vector4F left4 = this->getCameraMatrix() * Vector4F(-1, 0, 0, 0);
	return Vector3F(left4.getX(), left4.getY(), left4.getZ()).normalised();
}

Vector3F Camera::getRight() const
{
	// put the axis-aligned right vector into world space with swizzling to cull the w component
	Vector4F right4 = this->getCameraMatrix() * Vector4F(1, 0, 0, 0);
	return Vector3F(right4.getX(), right4.getY(), right4.getZ()).normalised();
}

Matrix4x4 Camera::getCameraMatrix() const
{
	// act as if the camera is an object to put into (camera-perspective) world space. but only apply the y-rotation if axis is bound.
	if(this->axis_bound)
		return Matrix4x4::createModelMatrix(this->pos, Vector3F(0, this->rot.getY(), 0), Vector3F(1, 1, 1));
	else
		return Matrix4x4::createModelMatrix(this->pos, this->rot, Vector3F(1, 1, 1));
}

bool Camera::isAxisBound() const
{
	return this->axis_bound;
}

void Camera::setAxisBound(bool axis_bound)
{
	this->axis_bound = axis_bound;
}