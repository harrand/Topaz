#include "resource_writer.hpp"
#include "core/matrix_transform.hpp"

namespace tz::gl
{
    TransformResourceWriter::TransformResourceWriter(tz::mem::Block data): mat_writer(data)
    {

    }

    bool TransformResourceWriter::write(tz::Vec3 pos, tz::Vec3 rot, tz::Vec3 scale, tz::Vec3 cam_pos, tz::Vec3 cam_rot, float fov, float aspect, float near, float far)
    {
        tz::Mat4 model = tz::model(pos, rot, scale);
        tz::Mat4 view = tz::view(cam_pos, cam_rot);
        tz::Mat4 proj = tz::perspective(fov, aspect, near, far);
        return this->mat_writer.write(proj * view * model);
    }

    void TransformResourceWriter::reset()
    {
        this->mat_writer.reset();
    }
}