#ifndef TOPAZ_GL_VK_SAMPLER_HPP
#define TOPAZ_GL_VK_SAMPLER_HPP
#if TZ_VULKAN
#include "gl/vk/logical_device.hpp"

namespace tz::gl::vk
{
    class Sampler
    {
    public:
        Sampler(const LogicalDevice& device);
        Sampler(const Sampler& copy) = delete;
        Sampler(Sampler&& move);
        ~Sampler();

        Sampler& operator=(const Sampler& rhs) = delete;
        Sampler& operator=(Sampler&& rhs);

        VkSampler native() const;
    private:
        VkSampler sampler;
        const LogicalDevice* device;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_SAMPLER_HPP