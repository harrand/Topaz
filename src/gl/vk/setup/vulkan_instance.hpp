#ifndef TOPAZ_GL_VK_SETUP_VULKAN_INSTANCE_HPP
#define TOPAZ_GL_VK_SETUP_VULKAN_INSTANCE_HPP
#if TZ_VULKAN
#include "core/game_info.hpp"
#include "gl/vk/setup/extension_list.hpp"
#include "gl/vk/validation/layer.hpp"
#include "vulkan/vulkan.h"

namespace tz::gl::vk
{
    class VulkanApplicationInfo
    {
    public:
        VulkanApplicationInfo(tz::GameInfo game_info);
        VkApplicationInfo get_info() const;
    private:
        std::string engine_name;
        VkApplicationInfo app_info;
    };

    class VulkanInstance
    {
    public:
        VulkanInstance(VulkanApplicationInfo vk_app_info, ExtensionList extensions = {}, ValidationLayerList validation_layers = {});
        ~VulkanInstance();
        VkInstance operator()() const;
        VkInstance native() const;
    private:
        VkInstance instance;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_SETUP_VULKAN_INSTANCE_HPP