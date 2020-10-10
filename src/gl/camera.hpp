#ifndef TOPAZ_GL_CAMERA_HPP
#define TOPAZ_GL_CAMERA_HPP
#include "algo/math.hpp"
#include "geo/vector.hpp"

namespace tz::core
{
    class IWindow;
}

namespace tz::gl
{
    struct CameraData
    {
        constexpr static float default_fov = tz::pi / 2.0f;
        constexpr static float default_near = 0.1f;
        constexpr static float default_far = 1000.0f;
        constexpr static float default_aspect_ratio = 1920.0f / 1080.0f;
        CameraData(const tz::core::IWindow& wnd, float fov = default_fov, float near = default_near, float far = default_far);
        CameraData() = default;
        tz::Vec3 position = {};
        tz::Vec3 rotation = {};
        float fov = default_fov;
        float aspect_ratio = default_aspect_ratio;
        float near = default_near;
        float far = default_far;
    };
}

#endif // TOPAZ_GL_CAMERA_HPP