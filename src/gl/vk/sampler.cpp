#if TZ_VULKAN
#include "gl/vk/sampler.hpp"

namespace tz::gl::vk
{
    Sampler::Sampler(const LogicalDevice& device):
    sampler(VK_NULL_HANDLE),
    device(&device)
    {
        VkSamplerCreateInfo create{};
        create.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        create.magFilter = VK_FILTER_LINEAR;
        create.minFilter = VK_FILTER_LINEAR;

        create.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        create.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        create.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

        create.anisotropyEnable = VK_FALSE;

        create.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        create.unnormalizedCoordinates = VK_FALSE;
        create.compareEnable = VK_FALSE;
        create.compareOp = VK_COMPARE_OP_ALWAYS;

        create.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        create.mipLodBias = 0.0f;
        create.minLod = 0.0f;
        create.maxLod = 0.0f;

        auto res = vkCreateSampler(this->device->native(), &create, nullptr, &this->sampler);
        tz_assert(res == VK_SUCCESS, "Failed to create sampler");
    }

    Sampler::Sampler(Sampler&& move):
    sampler(VK_NULL_HANDLE),
    device(nullptr)
    {
        *this = std::move(move);
    }

    Sampler::~Sampler()
    {
        if(this->sampler != VK_NULL_HANDLE)
        {
            vkDestroySampler(this->device->native(), this->sampler, nullptr);
            this->sampler = VK_NULL_HANDLE;
        }
    }

    Sampler& Sampler::operator=(Sampler&& rhs)
    {
        std::swap(this->sampler, rhs.sampler);
        std::swap(this->device, rhs.device);
        return *this;
    }

    VkSampler Sampler::native() const
    {
        return this->sampler;
    }
}

#endif // TZ_VULKAN