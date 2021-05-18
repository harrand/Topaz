#ifndef TOPAZ_GL_IMPL_COMMON_DEVICE_HPP
#define TOPAZ_GL_IMPL_COMMON_DEVICE_HPP

namespace tz::gl
{
    enum class GraphicsPrimitiveType
    {
        Triangles,
    };

    using DeviceWindowResizeCallback = std::function<void()>;
}

#endif // TOPAZ_GL_IMPL_COMMON_DEVICE_HPP