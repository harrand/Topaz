//
// Created by Harrand on 19/01/2020.
//

#ifndef TOPAZ_CORE_MATRIX_TRANSFORM_HPP
#define TOPAZ_CORE_MATRIX_TRANSFORM_HPP
#include "tz/core/matrix.hpp"
#include "hdk/data/vector.hpp"

namespace tz
{
	/**
	 * @ingroup tz_core_vecmat
	 * @defgroup tz_core_vecmat_transform matrix Transformations
	 * Documentation for matrix transformation functions.
	 */

	/**
	 * @ingroup tz_core_vecmat_transform
	 * Generate a matrix which performs the following transformations at once:
	 * - Translates the result position[0] units in the positive x-direction.
	 * - Translates the result position[1] units in the positive y-direction.
	 * - Translates the result position[2] units in the positive z-direction.
	 */
	mat4 translate(hdk::vec3 position);
	/**
	 * @ingroup tz_core_vecmat_transform
	 * Generates a matrix which performs the following three transformations, in chronological order:
	 * - Rotates the result rotation[0] radians about the x-axis.
	 * - Rotates the result rotation[1] radians about the y-axis.
	 * - Rotates the result rotation[2] radians about the z-axis.
	 */
	mat4 rotate(hdk::vec3 rotation);
	/**
	 * @ingroup tz_core_vecmat_transform
	 * Generates a matrix which performs the following transformations at once:
	 * - Multiplies each value by scale[0] in the x-direction.
	 * - Multiplies each value by scale[1] in the y-direction.
	 * - Multiplies each value by scale[2] in the z-direction.
	 */
	mat4 scale(hdk::vec3 scale);
	/**
	 * @ingroup tz_core_vecmat_transform
	 * Generates a model matrix using the given position, euler-rotation and scale vectors.
	 * Note: This can be used to transform positions within model-space to world-space.
	 */
	mat4 model(hdk::vec3 position, hdk::vec3 rotation, hdk::vec3 scale);
	/**
	 * @ingroup tz_core_vecmat_transform
	 * Generates a view matrix using the given view position and rotation.
	 * Note: This can be used to transform positions within world-space to camera-space.
	 */
	mat4 view(hdk::vec3 position, hdk::vec3 rotation);
	/**
	 * @ingroup tz_core_vecmat_transform
	 * Generates a perspective projection matrix using the given camera properties. The properties generate a regular pyramidal-frustum-shaped projection.
	 * Note: This can be used to transform positions within camera-space to clip-space.
	 */
	mat4 perspective(float fov, float aspect_ratio, float near, float far);
	/**
	 * @ingroup tz_core_vecmat_transform
	 * Generates an orthographic projection matrix using the given camera properties. The properties generate a cube-shaped projection.
	 */
	mat4 orthographic(float left, float right, float top, float bottom, float near, float far);
}

#endif // TOPAZ_CORE_MATRIX_TRANSFORM_HPP
