#ifndef CAMERA_HPP
#define CAMERA_HPP
#include "data/matrix.hpp"
#include "utility/numeric.hpp"

namespace tz::graphics
{
	/// FOV of a Camera if none is specified.
	constexpr float default_fov = tz::utility::numeric::consts::pi / 2.0f;
	/// Near-clip of a Camera if none is specified.
	constexpr float default_near_clip = 0.1f;
	/// Far-clip of a Camera if none is specified.
	constexpr float default_far_clip = 10000.0f;
}

/**
* Used to navigate the 3D scene such that all objects in the scene do not have to be moved in order to achieve motion.
*/
class Camera
{
public:
	/**
	 * Construct a Camera from all specifications.
	 * @param position - 3-dimensional position Vector representing the location of the Camera in world-space
	 * @param rotation - 3-dimensional orientation of the Camera, in euler-angles
	 * @param fov - Angle of field-of-view, in radians
	 * @param near_clip - Distance from the Camera to the near-plane
	 * @param far_clip - Distance from the Camera to the far-plane
	 * @param perspective - Whether the Camera should use a perspective projection
	 */
	Camera(Vector3F position = Vector3F(), Vector3F rotation = Vector3F(), float fov = tz::graphics::default_fov, float near_clip = tz::graphics::default_near_clip, float far_clip = tz::graphics::default_far_clip, bool perspective = true);
	/**
	 * Get the forward orientation.
	 * @return - Normalised 3-dimensional Vector representing 'forward' for the Camera
	 */
	Vector3F forward() const;
	/**
	 * Get the backward orientation.
	 * @return - Normalised 3-dimensional Vector representing 'backward' for the Camera
	 */
	Vector3F backward() const;
	/**
	 * Get the up orientation.
	 * @return - Normalised 3-dimensional Vector representing 'up' for the Camera
	 */
	Vector3F up() const;
	/**
	 * Get the down orientation.
	 * @return - Normalised 3-dimensional Vector representing 'down' for the Camera
	 */
	Vector3F down() const;
	/**
	 * Get the left orientation.
	 * @return - Normalised 3-dimensional Vector representing 'left' for the Camera
	 */
	Vector3F left() const;
	/**
	 * Get the right orientation.
	 * @return - Normalised 3-dimensional Vector representing 'right' for the Camera
	 */
	Vector3F right() const;
	/**
	 * Axis-Bound Camera causes orientation methods to stick to their normal axes. Also prevents rotation in the x and z axis.
	 * @return - True if the Camera is axis-bound. False otherwise
	 */
	bool is_axis_bound() const;
	/**
	 * Specify whether the Camera should be axis-bound.
	 * @param axis_bound - State representing whether the Camera should be axis-bound or not
	 */
	void set_axis_bound(bool axis_bound);
	/**
	 * Query whether the Camera uses a perspective projection as opposed to an orthographic one.
	 * @return - True if the Camera has a perspective projection. False otherwise
	 */
	bool has_perspective_projection() const;
	void set_orthographic(float right, float left, float top, float bottom, float near, float far);
	Matrix4x4 view() const;
	/**
	 * Create the Camera's projection matrix, given an aspect ratio which is (width / height).
	 * @param width - Width of the viewport.
	 * @param height - Height of the viewport.
	 * @return - The Camera's projection matrix. Can be either perspective or orthographic.
	 */
	Matrix4x4 projection(float width, float height) const;

	/// 3-dimensional Vector representing position of the Camera in world-space.
	Vector3F position;
	/// 3-dimensional Vector representing the orientation of the Camera in euler-angles.
	Vector3F rotation;
	/// Field-of-view in radians.
	float fov;
	/// Minimum distance from the camera for objects to render.
	float near_clip;
	/// Maximum distance from the camera for objects to render.
	float far_clip;
private:
	/**
	 * Specify whether the Camera should use a perspective projection or not.
	 * @param perspective - State representing whether the Camera should use a perspective projection or not
	 */
	void set_has_perspective_projection(bool perspective);
	/**
	 * Generate a model-matrix as if the Camera was an Object. The view matrix is equal to the inverse of this matrix.
	 * @return - Row-Major model-matrix of the Camera.
	 */
	Matrix4x4 camera_matrix() const;
	/// Stores whether the Camera is axis-bound. Axis-Bound Camera causes orientation methods to stick to their normal axes. Also prevents rotation in the x and z axis.
	bool axis_bound;
	/// Stores whether the Camera is using a perspective projection as opposed to an orthographic projection.
	bool perspective;
	float ortho_right, ortho_left, ortho_top, ortho_bottom, ortho_near, ortho_far;
};

#endif