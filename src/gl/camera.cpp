#include "gl/camera.hpp"
#include "core/window.hpp"

namespace tz::gl
{
    CameraData::CameraData(const tz::core::IWindow& wnd, float fov, float near, float far): fov(fov), aspect_ratio(static_cast<float>(wnd.get_width()) / wnd.get_height()), near(near), far(far){}
}