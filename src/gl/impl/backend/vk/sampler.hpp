#ifndef TOPAZ_GL_VK_SAMPLER_HPP
#define TOPAZ_GL_VK_SAMPLER_HPP
#if TZ_VULKAN
#include "gl/impl/backend/vk/logical_device.hpp"

namespace tz::gl::vk
{
    struct SamplerProperties
    {   
        VkFilter min_filter;
        VkFilter mag_filter;

        VkSamplerAddressMode address_mode_u;
        VkSamplerAddressMode address_mode_v;
        VkSamplerAddressMode address_mode_w;
    };

    class Sampler
    {
    public:
        Sampler(const LogicalDevice& device, SamplerProperties props);
        Sampler(const Sampler& copy) = delete;
        Sampler(Sampler&& move);
        ~Sampler();

        Sampler& operator=(const Sampler& rhs) = delete;
        Sampler& operator=(Sampler&& rhs);

        const SamplerProperties& get_properties() const;
        VkSampler native() const;
    private:
        VkSampler sampler;
        const LogicalDevice* device;
        SamplerProperties properties;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_SAMPLER_HPP