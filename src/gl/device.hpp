#ifndef TOPAZ_GL_DEVICE_HPP
#define TOPAZ_GL_DEVICE_HPP

#if TZ_VULKAN
#include "gl/impl/frontend/vk/device.hpp"
namespace tz::gl
{
    using Device = DeviceVulkan;
    using DeviceBuilder = DeviceBuilderVulkan;
}
#elif TZ_OGL
#include "gl/impl/frontend/ogl/device.hpp"
namespace tz::gl
{
    using Device = DeviceOGL;
    using DeviceBuilder = DeviceBuilderOGL;
}
#endif

#endif // TOPAZ_GL_DEVICE_HPP