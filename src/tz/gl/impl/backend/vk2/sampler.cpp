#if TZ_VULKAN
#include "tz/gl/impl/backend/vk2/sampler.hpp"

namespace tz::gl::vk2
{
	Sampler::Sampler(SamplerInfo info):
	sampler(VK_NULL_HANDLE),
	info(info)
	{
		VkSamplerCreateInfo create
		{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.magFilter = static_cast<VkFilter>(this->info.mag_filter),
			.minFilter = static_cast<VkFilter>(this->info.min_filter),
			.mipmapMode = static_cast<VkSamplerMipmapMode>(this->info.mipmap_mode),
			.addressModeU = static_cast<VkSamplerAddressMode>(this->info.address_mode_u),
			.addressModeV = static_cast<VkSamplerAddressMode>(this->info.address_mode_v),
			.addressModeW = static_cast<VkSamplerAddressMode>(this->info.address_mode_w),
			.mipLodBias = 0.0f,
			.anisotropyEnable = VK_FALSE,
			.maxAnisotropy = 0.0f,
			.compareEnable = VK_FALSE,
			.compareOp = VK_COMPARE_OP_ALWAYS,
			.minLod = 0.0f,
			.maxLod = 0.0f,
			.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
			.unnormalizedCoordinates = VK_FALSE
		};

		VkResult res = vkCreateSampler(this->get_device().native(), &create, nullptr, &this->sampler);
		
		switch(res)
		{
			case VK_SUCCESS:
				// do nothing
			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				tz_error("Failed to create Sampler because we ran out of host memory (RAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				tz_error("Failed to create Sampler because we ran out of device memory (VRAM). Please ensure that your system meets the minimum requirements.");
			break;
			default:
				tz_error("Failed to create DescriptorSetLayout but cannot determine why. Please submit a bug report.");
			break;
		}
	}

	Sampler::Sampler(Sampler&& move):
	sampler(VK_NULL_HANDLE),
	info()
	{
		*this = std::move(move);
	}

	Sampler::~Sampler()
	{
		if(this->sampler != VK_NULL_HANDLE)
		{
			vkDestroySampler(this->get_device().native(), this->sampler, nullptr);
			this->sampler = VK_NULL_HANDLE;
		}
	}

	Sampler& Sampler::operator=(Sampler&& rhs)
	{
		std::swap(this->sampler, rhs.sampler);
		std::swap(this->info, rhs.info);
		return *this;
	}

	const LogicalDevice& Sampler::get_device() const
	{
		tz_assert(this->info.device != nullptr && !this->info.device->is_null(), "SamplerInfo contained nullptr or null LogicalDevice. Please submit a bug report.");
		return *this->info.device;
	}

	Sampler::NativeType Sampler::native() const
	{
		return this->sampler;
	}
}

#endif // TZ_VULKAN
