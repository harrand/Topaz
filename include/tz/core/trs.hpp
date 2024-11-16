#ifndef TOPAZ_CORE_TRS_HPP
#define TOPAZ_CORE_TRS_HPP
#include "tz/core/vector.hpp"
#include "tz/core/quaternion.hpp"
#include "tz/core/matrix.hpp"

namespace tz
{
	/**
	 * @ingroup tz_core
	 * @defgroup tz_core_transform Transforms
	 * @brief Represent transformations and hierarchies of transformations in 3D space.
	 */

	/**
	 * @ingroup tz_core_transform
	 * @brief Create a matrix that performs the given translation in 3D space.
	 */
	tz::m4f matrix_translate(tz::v3f translate);
	/**
	 * @ingroup tz_core_transform
	 * @brief Create a matrix that performs the given scale in 3D space.
	 */
	tz::m4f matrix_scale(tz::v3f scale);
	/**
	 * @ingroup tz_core_transform
	 * @brief Create an orthographic projection matrix.
	 */
	tz::m4f matrix_ortho(float left, float right, float top, float bottom, float near, float far);
	/**
	 * @ingroup tz_core_transform
	 * @brief Create a perspective projection matrix.
	 */
	tz::m4f matrix_persp(float fov, float aspect_ratio, float near, float far);
	/**
	 * @ingroup tz_core_transform
	 * @brief Create a perspective projection matrix with an infinitely-distant far plane.
	 */
	tz::m4f matrix_persp_nofar(float fov, float aspect_ratio, float near);

	/**
	 * @ingroup tz_core_transform
	 * @brief Encapsulates a combination of a Translation, Rotation, and Scale in 3D space.
	 */
	struct trs
	{
		/// Translation Component
		tz::v3f translate = tz::v3f::zero();
		/// Rotation Component
		tz::quat rotate = tz::quat::iden();
		/// Scale Component
		tz::v3f scale = tz::v3f::filled(1.0f);

		/// Linearly interpolate between this transform and another based on a factor, and return that result.
		trs lerp(const trs& rhs, float factor) const;
		/// Create a matrix that performs an identical transformation.
		m4f matrix() const;
		/// Create a new TRS that performs the identical transformation to the given matrix.
		static trs from_matrix(m4f mat);

		/// Return an inverse transform that does the exact opposite (such that multiplying a transform by its inverse yields no transformation).
		trs inverse() const;
		/// Combine one transform with another, and return the result. @note TRS combine operations, like matrix multiplications, are not commutative.
		trs combine(const trs& rhs);

		bool operator==(const trs& rhs) const = default;
	};
}

#endif // TOPAZ_CORE_TRS_HPP