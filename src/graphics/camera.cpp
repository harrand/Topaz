#include "graphics/camera.hpp"

Camera::Camera(Vector3F position, Vector3F rotation, float fov, float near_clip, float far_clip, bool perspective): position(position), rotation(rotation), fov(fov), near_clip(near_clip), far_clip(far_clip), axis_bound(false), perspective(perspective){}

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
		return tz::transform::model(this->position, Vector3F(0, this->rotation.y, 0), Vector3F(1, 1, 1));
	else
		return tz::transform::model(this->position, this->rotation, Vector3F(1, 1, 1));
}

bool Camera::is_axis_bound() const
{
	return this->axis_bound;
}

void Camera::set_axis_bound(bool axis_bound)
{
	this->axis_bound = axis_bound;
}

bool Camera::has_perspective_projection() const
{
	return this->perspective;
}

void Camera::set_has_perspective_projection(bool perspective)
{
	this->perspective = perspective;
}

Matrix4x4 Camera::view() const
{
	return tz::transform::view(this->position, this->rotation);
}

Matrix4x4 Camera::projection(float width, float height) const
{
	if(this->perspective)
		return tz::transform::perspective_projection(this->fov, width, height, this->near_clip, this->far_clip);
	else
	{
		float zoom = (width + height) * 0.25f * this->fov / tz::utility::numeric::consts::pi;
		float aspect_ratio = width / height;
		if (width >= height)
			return tz::transform::orthographic_projection(zoom * aspect_ratio, -zoom * aspect_ratio, zoom, -zoom, this->near_clip, this->far_clip);
		else
			return tz::transform::orthographic_projection(zoom, -zoom, zoom / aspect_ratio, -zoom / aspect_ratio, this->near_clip, this->far_clip);
	}
}