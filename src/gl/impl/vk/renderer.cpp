#if TZ_VULKAN
#include "gl/impl/vk/renderer.hpp"

namespace tz::gl
{
    RendererVulkan::RendererVulkan(RendererBuilderVulkan builder, RendererBuilderDeviceInfoVulkan device_info):
    input_assembly(device_info.primitive_type)
    {

    }
}
#endif // TZ_VULKAN