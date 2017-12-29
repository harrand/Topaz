#ifndef CAMERA_HPP
#define CAMERA_HPP
#include "matrix.hpp"
#include "utility.hpp"

namespace tz::graphics
{
	constexpr float default_fov = tz::consts::pi / 2.0f;
	constexpr float default_near_clip = 0.1f;
	constexpr float default_far_clip = 10000.0f;
}

/*
	Used to navigate the 3D scene such that all objects in the scene do not have to be moved in order to achieve motion.
*/
class Camera
{
public:
	Camera(Vector3F position = Vector3F(), Vector3F rotation = Vector3F(), float fov = tz::graphics::default_fov, float near_clip = tz::graphics::default_near_clip, float far_clip = tz::graphics::default_far_clip, bool perspective = true);
	Camera(const Camera& copy) = default;
	Camera(Camera&& move) = default;
	Camera& operator=(const Camera& rhs) = default;

	// Orientation methods.
	Vector3F forward() const;
	Vector3F backward() const;
	Vector3F up() const;
	Vector3F down() const;
	Vector3F left() const;
	Vector3F right() const;
	// Axis-Bound Camera causes orientation methods to stick to their normal axes. Also prevents rotation in the x and z axis.
	bool is_axis_bound() const;
	void set_axis_bound(bool axis_bound);
	bool has_perspective_projection() const;
	void set_has_perspective_projection(bool perspective);
	Matrix4x4 projection(float width, float height) const;
	
	Vector3F position, rotation;
	float fov, near_clip, far_clip;
private:
	Matrix4x4 camera_matrix() const;
	bool axis_bound, perspective;
};

#endif