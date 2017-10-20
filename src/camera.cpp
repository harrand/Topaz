#include "camera.hpp"

Camera::Camera(Vector3F pos, Vector3F rot, float fov, float near_clip, float far_clip): pos(std::move(pos)), rot(std::move(rot)), fov(fov), near_clip(near_clip), far_clip(far_clip){}

const Vector3F& Camera::get_position() const
{
	return this->pos;
}

const Vector3F& Camera::get_rotation() const
{
	return this->rot;
}

void Camera::set_position(Vector3F position)
{
	this->pos = position;
}

void Camera::set_rotation(Vector3F rotation)
{
	this->rot = rotation;
}

float Camera::get_fov() const
{
	return this->fov;
}

float Camera::get_near_clip() const
{
	return this->near_clip;
}

float Camera::get_far_clip() const
{
	return this->far_clip;
}

void Camera::set_fov(float fov)
{
	this->fov = fov;
}

void Camera::set_near_clip(float near_clip)
{
	this->near_clip = near_clip;
}

void Camera::set_far_clip(float far_clip)
{
	this->far_clip = far_clip;
}

Vector3F Camera::forward() const
{
	// put the axis-aligned forward vector into world space with swizzling to cull the w component
	Vector4F forward_4 = this->camera_matrix() * Vector4F(0, 0, -1, 0);
	return forward_4.xyz().normalised();
}

Vector3F Camera::backward() const
{
	// put the axis-aligned backward vector into world space with swizzling to cull the w component
	Vector4F backward_4 = this->camera_matrix() * Vector4F(0, 0, 1, 0);
	return backward_4.xyz().normalised();
}

Vector3F Camera::up() const
{
	// put the axis-aligned upward vector into world space with swizzling to cull the w component
	Vector4F up_4 = this->camera_matrix() * Vector4F(0, 1, 0, 0);
	return up_4.xyz().normalised();
}

Vector3F Camera::down() const
{
	// put the axis-aligned downward vector into world space with swizzling to cull the w component
	Vector4F down_4 = this->camera_matrix() * Vector4F(0, -1, 0, 0);
	return down_4.xyz().normalised();
}

Vector3F Camera::left() const
{
	// put the axis-aligned left vector into world space with swizzling to cull the w component
	Vector4F left_4 = this->camera_matrix() * Vector4F(-1, 0, 0, 0);
	return left_4.xyz().normalised();
}

Vector3F Camera::right() const
{
	// put the axis-aligned right vector into world space with swizzling to cull the w component
	Vector4F right_4 = this->camera_matrix() * Vector4F(1, 0, 0, 0);
	return right_4.xyz().normalised();
}

Matrix4x4 Camera::camera_matrix() const
{
	// act as if the camera is an object to put into (camera-perspective) world space. but only apply the y-rotation if axis is bound.
	if(this->axis_bound)
		return Matrix4x4::create_model_matrix(this->pos, Vector3F(0, this->rot.get_y(), 0), Vector3F(1, 1, 1));
	else
		return Matrix4x4::create_model_matrix(this->pos, this->rot, Vector3F(1, 1, 1));
}

bool Camera::is_axis_bound() const
{
	return this->axis_bound;
}

void Camera::set_axis_bound(bool axis_bound)
{
	this->axis_bound = axis_bound;
}