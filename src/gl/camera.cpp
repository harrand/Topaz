#include "gl/camera.hpp"
#include "core/window.hpp"

namespace tz::gl
{
    CameraData::CameraData(const tz::gl::IFrame& frame, float fov, float near, float far): fov(fov), aspect_ratio(static_cast<float>(frame.get_width()) / frame.get_height()), near(near), far(far){}
    CameraData::CameraData()
    {
        this->aspect_ratio = static_cast<float>(tz::ext::glfw::get_monitor_width()) / tz::ext::glfw::get_monitor_height();
    }
}