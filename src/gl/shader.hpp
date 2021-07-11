#ifndef TOPAZ_GL_SHADER_HPP
#define TOPAZ_GL_SHADER_HPP

#if TZ_VULKAN
#include "gl/impl/frontend/vk/shader.hpp"
namespace tz::gl
{
    using Shader = ShaderVulkan;
    using ShaderBuilder = ShaderBuilderVulkan;
}
#elif TZ_OGL
#include "gl/impl/frontend/ogl/shader.hpp"
namespace tz::gl
{
    using Shader = ShaderOGL;
    using ShaderBuilder = ShaderBuilderOGL;
}
#endif

#endif // TOPAZ_GL_DEVICE_HPP