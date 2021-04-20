#if TZ_VULKAN
#include "gl/vk/impl/setup/extension_list.hpp"
#include "gl/vk/impl/validation/layer.hpp"
#include "GLFW/glfw3.h"
#include "vulkan/vulkan.h"
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

    ExtensionList get_default_required_extensions()
    {
        ExtensionList exts = get_glfw_required_extensions();
        if constexpr(tz::gl::vk::validation::layers_enabled)
        {
            exts.add(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // debug messenger
        }
        return exts;
    }
}

#endif