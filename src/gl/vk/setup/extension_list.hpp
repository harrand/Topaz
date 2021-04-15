#ifndef TOPAZ_GL_VK_SETUP_EXTENSION_LIST_HPP
#define TOPAZ_GL_VK_SETUP_EXTENSION_LIST_HPP
#if TZ_VULKAN
#include <vector>

namespace tz::gl::vk
{
    using VulkanExtension = const char*;

    class ExtensionList
    {
    public:
        ExtensionList(std::initializer_list<VulkanExtension> extensions = {}):
        extensions(extensions){}

        void add(VulkanExtension extension)
        {
            this->extensions.push_back(extension);
        }

        void append(const ExtensionList& other)
        {
            this->extensions.insert(this->extensions.end(), other.extensions.begin(), other.extensions.end());
        }

        auto length() const
        {
            return this->extensions.size();
        }

        const VulkanExtension* data() const
        {
            return this->extensions.data();
        }
    private:
        std::vector<VulkanExtension> extensions;
    };

    ExtensionList get_glfw_required_extensions();
}

#endif // TZ_VULKAn
#endif // TOPAZ_GL_VK_SETUP_EXTENSION_LIST_HPP