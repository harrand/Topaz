#ifndef TOPAZ_GL_CAMERA_HPP
#define TOPAZ_GL_CAMERA_HPP
#include "algo/math.hpp"
#include "geo/vector.hpp"
#include "gl/frame.hpp"

namespace tz::core
{
    class IWindow;
}

namespace tz::gl
{
    /**
	 * \addtogroup tz_gl Topaz Graphics Library (tz::gl)
	 * @{
	 */
    
    struct CameraData
    {
        // Default field-of-view for all Cameras, in radians.
        constexpr static float default_fov = tz::pi / 2.0f;
        // Default distance between near-clip plane and camera position.
        constexpr static float default_near = 0.1f;
        // Default distance between far-clip plane and camera position.
        constexpr static float default_far = 1000.0f;
        /**
         * Construct camera data using the aspect-ratio of an existing frame.
         * @param frame Frame whose current aspect-ratio shall be used.
         * @param fov Field-of-view, in radians.
         * @param near Distance between near-plane and camera position.
         * @param far Distance between far-plane and camera position.
         */
        CameraData(const tz::gl::IFrame& frame, float fov = default_fov, float near = default_near, float far = default_far);
        /**
         * Construct camera data using default values. The aspect-ratio will be set to the aspect ratio of the primary monitor.
         */
        CameraData();
        tz::Vec3 position = {};
        tz::Vec3 rotation = {};
        float fov = default_fov;
        float aspect_ratio;
        float near = default_near;
        float far = default_far;
    };

    /**
     * @}
     */
}

#endif // TOPAZ_GL_CAMERA_HPP