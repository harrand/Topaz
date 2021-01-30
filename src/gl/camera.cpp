#include "gl/camera.hpp"
#include "gl/screen.hpp"
#include "core/matrix_transform.hpp"

namespace tz::gl
{
    CameraData::CameraData(const tz::gl::IFrame& frame, float fov, float near, float far): fov(fov), aspect_ratio(static_cast<float>(frame.get_width()) / frame.get_height()), near(near), far(far){}
    CameraData::CameraData()
    {
        this->aspect_ratio = tz::gl::Screen::primary().get_aspect_ratio();
    }

    tz::Vec3 CameraData::get_forward() const
    {
        return this->camera_dir({0.0f, 0.0f, -1.0f});
    }

    tz::Vec3 CameraData::get_backward() const
    {
        return this->camera_dir({0.0f, 0.0f, 1.0f});
    }

    tz::Vec3 CameraData::get_left() const
    {
        return this->camera_dir({-1.0f, 0.0f, 0.0f});
    }

    tz::Vec3 CameraData::get_right() const
    {
        return this->camera_dir({1.0f, 0.0f, 0.0f});
    }

    tz::Vec3 CameraData::get_up() const
    {
        return this->camera_dir({0.0f, 1.0f, 0.0f});
    }

    tz::Vec3 CameraData::get_down() const
    {
        return this->camera_dir({0.0f, -1.0f, 0.0f});
    }

    tz::Vec3 CameraData::camera_dir(tz::Vec3 direction) const
    {
        tz::Vec4 dir_homogeneous{direction[0], direction[1], direction[2], 0.0f};
        tz::Mat4 camera_matrix = tz::view(this->position, this->rotation);
        tz::Vec4 res = camera_matrix * dir_homogeneous;
        return {res[0], res[1], res[2]};
    }
}