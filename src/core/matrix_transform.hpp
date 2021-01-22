//
// Created by Harrand on 19/01/2020.
//

#ifndef TOPAZ_CORE_MATRIX_TRANSFORM_HPP
#define TOPAZ_CORE_MATRIX_TRANSFORM_HPP
#include "core/matrix.hpp"
#include "core/vector.hpp"

namespace tz
{
	/**
	 * \addtogroup tz_core Topaz Core Library (tz::core)
	 * A collection of platform-agnostic core interfaces.
	 * @{
	 */

	/**
	 * \addtogroup tz_core_mat tz::core Matrix Transformation Module
	 * Implementations of various matrix transformations. Note that all resultant matrices are row-major.
	 * @{
	 */

	/**
	 * Generate a matrix which performs the following transformations at once:
	 * - Translates the result position[0] units in the positive x-direction.
	 * - Translates the result position[1] units in the positive y-direction.
	 * - Translates the result position[2] units in the positive z-direction.
	 */
	Mat4 translate(Vec3 position);
	/**
	 * Generates a matrix which performs the following three transformations, in chronological order:
	 * - Rotates the result rotation[0] radians about the x-axis.
	 * - Rotates the result rotation[1] radians about the y-axis.
	 * - Rotates the result rotation[2] radians about the z-axis.
	 */
	Mat4 rotate(Vec3 rotation);
	/**
	 * Generates a matrix which performs the following transformations at once:
	 * - Multiplies each value by scale[0] in the x-direction.
	 * - Multiplies each value by scale[1] in the y-direction.
	 * - Multiplies each value by scale[2] in the z-direction.
	 */
	Mat4 scale(Vec3 scale);

	/**
	 * Generates a model matrix using the given position, euler-rotation and scale vectors.
	 * Note: This can be used to transform positions within model-space to world-space.
	 */
	Mat4 model(Vec3 position, Vec3 rotation, Vec3 scale);
	/**
	 * Generates a view matrix using the given view position and rotation.
	 * Note: This can be used to transform positions within world-space to camera-space.
	 */
	Mat4 view(Vec3 position, Vec3 rotation);
	/**
	 * Generates a perspective projection matrix using the given camera properties. The properties generate a regular pyramidal-frustum-shaped projection.
	 * Note: This can be used to transform positions within camera-space to clip-space.
	 */
	Mat4 perspective(float fov, float aspect_ratio, float near, float far);
	/**
	 * Generates an orthographic projection matrix using the given camera properties. The properties generate a cube-shaped projection.
	 */
	Mat4 orthographic(float left, float right, float top, float bottom, float near, float far);

	/**
	 * @}
	 */
	/**
	 * @}
	 */
}

#endif // TOPAZ_CORE_MATRIX_TRANSFORM_HPP