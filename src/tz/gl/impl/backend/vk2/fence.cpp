#if TZ_VULKAN
#include "tz/gl/impl/backend/vk2/fence.hpp"

namespace tz::gl::vk2
{
	Fence::Fence(FenceInfo info):
	DebugNameable<VK_OBJECT_TYPE_FENCE>(*info.device),
	fence(VK_NULL_HANDLE),
	info(info)
	{
		VkFenceCreateInfo create
		{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.pNext = nullptr,
			.flags = info.initially_signalled ? static_cast<VkFenceCreateFlags>(VK_FENCE_CREATE_SIGNALED_BIT) : static_cast<VkFenceCreateFlags>(0),
		};

		VkResult res = vkCreateFence(this->get_device().native(), &create, nullptr, &this->fence);
		switch(res)
		{
			case VK_SUCCESS:
				// do nothing
			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				hdk::error("Failed to create Fence because we ran out of host memory (RAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				hdk::error("Failed to create Fence because we ran out of device memory (VRAM). Please ensure that your system meets the minimum requirements.");
			break;
			default:
				hdk::error("Failed to create Fence but cannot determine why. Please submit a bug report.");
			break;
		}
		DebugNameable<VK_OBJECT_TYPE_FENCE>::debug_set_handle(reinterpret_cast<std::uint64_t>(this->fence));
	}

	Fence::Fence(Fence&& move):
	fence(VK_NULL_HANDLE),
	info()
	{
		*this = std::move(move);
	}

	Fence::~Fence()
	{
		if(this->fence != VK_NULL_HANDLE)
		{
			vkDestroyFence(this->get_device().native(), this->fence, nullptr);
			this->fence = VK_NULL_HANDLE;
		}
	}

	Fence& Fence::operator=(Fence&& rhs)
	{
		std::swap(this->fence, rhs.fence);
		std::swap(this->info, rhs.info);
		DebugNameable<VK_OBJECT_TYPE_FENCE>::debugname_swap(rhs);
		return *this;
	}

	const LogicalDevice& Fence::get_device() const
	{
		hdk::assert(this->info.device != nullptr && !this->info.device->is_null(), "Fence contained nullptr or null LogicalDevice. Please submit a bug report.");
		return *this->info.device;
	}

	bool Fence::is_signalled() const
	{
		return vkGetFenceStatus(this->get_device().native(), this->fence) == VK_SUCCESS;
	}

	void Fence::wait_until_signalled() const
	{
		vkWaitForFences(this->get_device().native(), 1, &this->fence, VK_FALSE, std::numeric_limits<std::uint64_t>::max());
	}

	void Fence::unsignal()
	{
		vkResetFences(this->get_device().native(), 1, &this->fence);
	}

	Fence::NativeType Fence::native() const
	{
		return this->fence;
	}
}

#endif // TZ_VULKAN
