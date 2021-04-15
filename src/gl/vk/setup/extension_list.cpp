#if TZ_VULKAN
#include "gl/vk/setup/extension_list.hpp"
#include "GLFW/glfw3.h"
#include <cstdint>

namespace tz::gl::vk
{
    ExtensionList get_glfw_required_extensions()
    {
        std::uint32_t glfw_extension_count = 0;
        const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

        ExtensionList glfw_vulkan_extensions;
        for(auto i = 0; i < glfw_extension_count; i++)
        {
            glfw_vulkan_extensions.add(glfw_extensions[i]);
        }

        return glfw_vulkan_extensions;
    }
}

#endif