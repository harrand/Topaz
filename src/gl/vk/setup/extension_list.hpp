#ifndef TOPAZ_GL_VK_SETUP_EXTENSION_LIST_HPP
#define TOPAZ_GL_VK_SETUP_EXTENSION_LIST_HPP
#if TZ_VULKAN
#include "gl/vk/common/basic_list.hpp"

namespace tz::gl::vk
{
    using VulkanExtension = const char*;

    using ExtensionList = common::BasicList<VulkanExtension>;

    ExtensionList get_glfw_required_extensions();
}

#endif // TZ_VULKAn
#endif // TOPAZ_GL_VK_SETUP_EXTENSION_LIST_HPP