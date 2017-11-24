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
	Used to navigate the 3D world such that all objects in the world do not have to be moved in order to achieve motion.
*/
class Camera
{
public:
	Camera(Vector3F pos = Vector3F(), Vector3F rot = Vector3F(), float fov = tz::graphics::default_fov, float near_clip = tz::graphics::default_near_clip, float far_clip = tz::graphics::default_far_clip);
	Camera(const Camera& copy) = default;
	Camera(Camera&& move) = default;
	Camera& operator=(const Camera& rhs) = default;
	
	const Vector3F& get_position() const;
	const Vector3F& get_rotation() const;
	void set_position(Vector3F position);
	void set_rotation(Vector3F rotation);
	float get_fov() const;
	float get_near_clip() const;
	float get_far_clip() const;
	void set_fov(float fov);
	void set_near_clip(float near_clip);
	void set_far_clip(float far_clip);
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
private:
	Matrix4x4 camera_matrix() const;
	Vector3F pos, rot;
	float fov, near_clip, far_clip;
	bool axis_bound;
};

#endif