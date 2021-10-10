#if TZ_VULKAN
#include "core/profiling/zone.hpp"
#include "gl/impl/backend/vk/fence.hpp"

namespace tz::gl::vk
{
	Fence::Fence(const LogicalDevice& device):
	fence(VK_NULL_HANDLE),
	device(&device)
	{
		VkFenceCreateInfo create{};
		create.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		create.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		auto res = vkCreateFence(this->device->native(), &create, nullptr, &this->fence);
		tz_assert(res == VK_SUCCESS, "Failed to create fence");
	}

	Fence::Fence(Fence&& move):
	fence(VK_NULL_HANDLE),
	device(nullptr)
	{
		*this = std::move(move);
	}

	Fence::~Fence()
	{
		if(this->fence != VK_NULL_HANDLE)
		{
			vkDestroyFence(this->device->native(), this->fence, nullptr);
			this->fence = VK_NULL_HANDLE;
		}
	}

	Fence& Fence::operator=(Fence&& rhs)
	{
		std::swap(this->fence, rhs.fence);
		std::swap(this->device, rhs.device);
		return *this;
	}

	VkFence Fence::native() const
	{
		return this->fence;
	}

	void Fence::wait_for() const
	{
		TZ_PROFZONE("Backend VK : Fence Wait", TZ_PROFCOL_RED);
		vkWaitForFences(this->device->native(), 1, &this->fence, VK_TRUE, std::numeric_limits<std::uint64_t>::max());
	}

	void Fence::signal() const
	{ 
		vkResetFences(this->device->native(), 1, &this->fence);
	}

	void Fence::wait_then_signal() const
	{
		this->wait_for();
		this->signal();
	}
}

#endif // TZ_VULKAN