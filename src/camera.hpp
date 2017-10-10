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

class Camera
{
public:
	Camera(Vector3F pos = Vector3F(), Vector3F rot = Vector3F(), float fov = tz::graphics::default_fov, float near_clip = tz::graphics::default_near_clip, float far_clip = tz::graphics::default_far_clip);
	Camera(const Camera& copy) = default;
	Camera(Camera&& move) = default;
	Camera& operator=(const Camera& rhs) = default;
	
	const Vector3F& getPosition() const;
	const Vector3F& getRotation() const;
	Vector3F& getPositionR();
	Vector3F& getRotationR();
	float getFOV() const;
	float getNearClip() const;
	float getFarClip() const;
	float& getFOVR();
	float& getNearClipR();
	float& getFarClipR();
	Vector3F getForward() const;
	Vector3F getBackward() const;
	Vector3F getUp() const;
	Vector3F getDown() const;
	Vector3F getLeft() const;
	Vector3F getRight() const;
	bool isAxisBound() const;
	void setAxisBound(bool axis_bound);
private:
	Matrix4x4 getCameraMatrix() const;
	Vector3F pos, rot;
	float fov, near_clip, far_clip;
	bool axis_bound;
};

#endif