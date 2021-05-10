#ifndef TOPAZ_GL_DEVICE_HPP
#define TOPAZ_GL_DEVICE_HPP
#include "gl/render_pass.hpp"

#if TZ_VULKAN
#include "gl/impl/vk/device.hpp"
namespace tz::gl
{
    using Device = DeviceVulkan;
}
#elif TZ_OGL
#include "gl/impl/ogl/device.hpp"
namespace tz::gl
{
    using Device = DeviceOGL;
}
#endif

#endif // TOPAZ_GL_DEVICE_HPP