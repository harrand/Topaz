#ifndef TOPAZ_GL_IMPL_VK_RENDERER_HPP
#define TOPAZ_GL_IMPL_VK_RENDERER_HPP
#include "gl/vk/pipeline/input_assembly.hpp"

namespace tz::gl
{
    class RendererBuilderVulkan
    {

    };

    struct RendererBuilderDeviceInfoVulkan
    {
        vk::pipeline::PrimitiveTopology primitive_type;
    };

    class RendererVulkan
    {
    public:
        RendererVulkan(RendererBuilderVulkan builder, RendererBuilderDeviceInfoVulkan);
    private:
        vk::pipeline::InputAssembly input_assembly;
    };
}

#endif // TOPAZ_GL_IMPL_VK_RENDERER_HPP