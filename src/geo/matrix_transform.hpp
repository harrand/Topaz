//
// Created by Harrand on 19/01/2020.
//

#ifndef TOPAZ_GEO_MATRIX_TRANSFORM_HPP
#define TOPAZ_GEO_MATRIX_TRANSFORM_HPP
#include "geo/matrix.hpp"
#include "geo/vector.hpp"

namespace tz::geo
{
    /**
     * \addtogroup tz_geo Topaz Geometry Library (tz::geo)
     * A collection of geometric data structures and mathematical types, such as vectors and matrices.
     * @{
     */

    /**
     * \addtogroup tz_geo_mat tz::geo Matrix Transformation Module
     * Implementations of various matrix transformations.
     * @{
     */

    Mat4 translate(Vec3 position);
    Mat4 rotate(Vec3 rotation);
    Mat4 scale(Vec3 scale);

    Mat4 model(Vec3 position, Vec3 rotation, Vec3 scale);
    Mat4 view(Vec3 position, Vec3 rotation);
    Mat4 perspective(float fov, float aspect_ratio, float near, float far);
    Mat4 orthographic(float left, float right, float top, float bottom, float near, float far);

    /**
     * @}
     */
    /**
     * @}
     */
}

#endif // TOPAZ_GEO_MATRIX_TRANSFORM_HPP