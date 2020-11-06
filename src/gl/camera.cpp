#include "gl/camera.hpp"
#include "gl/screen.hpp"

namespace tz::gl
{
    CameraData::CameraData(const tz::gl::IFrame& frame, float fov, float near, float far): fov(fov), aspect_ratio(static_cast<float>(frame.get_width()) / frame.get_height()), near(near), far(far){}
    CameraData::CameraData()
    {
        this->aspect_ratio = tz::gl::Screen::primary().get_aspect_ratio();
    }
}