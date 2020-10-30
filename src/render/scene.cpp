#include "render/scene.hpp"

namespace tz::render
{
    tz::gl::Transform SceneElement::get_transform() const
    {
        return this->transform;
    }

    tz::gl::CameraData SceneElement::get_camera_data() const
    {
        return this->camera;
    }

    tz::render::AssetBuffer::Index SceneElement::get_mesh_index() const
    {
        return this->mesh;
    }
}