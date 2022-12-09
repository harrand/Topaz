#if TZ_VULKAN
#include "tz/gl/impl/backend/vk2/semaphore.hpp"

namespace tz::gl::vk2
{
	BinarySemaphore::BinarySemaphore(const LogicalDevice& device):
	DebugNameable<VK_OBJECT_TYPE_SEMAPHORE>(device),
	sem(VK_NULL_HANDLE),
	device(&device)
	{
		VkSemaphoreCreateInfo create
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0
		};

		VkResult res = vkCreateSemaphore(this->device->native(), &create, nullptr, &this->sem);
		switch(res)
		{
			case VK_SUCCESS:
				// do nothing
			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				hdk::error("Failed to create BinarySemaphore because we ran out of host memory (RAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				hdk::error("Failed to create BinarySemaphore because we ran out of device memory (VRAM). Please ensure that your system meets the minimum requirements.");
			break;
			default:
				hdk::error("Failed to create BinarySemaphore but cannot determine why. Please submit a bug report.");
			break;
		}
		DebugNameable<VK_OBJECT_TYPE_SEMAPHORE>::debug_set_handle(reinterpret_cast<std::uint64_t>(this->sem));
	}

	BinarySemaphore::BinarySemaphore(BinarySemaphore&& move):
	BinarySemaphore()
	{
		*this = std::move(move);
	}

	BinarySemaphore::~BinarySemaphore()
	{
		if(this->sem != VK_NULL_HANDLE)
		{
			vkDestroySemaphore(this->device->native(), this->sem, nullptr);
			this->sem = VK_NULL_HANDLE;
		}
	}

	BinarySemaphore& BinarySemaphore::operator=(BinarySemaphore&& rhs)
	{
		std::swap(this->sem, rhs.sem);
		std::swap(this->device, rhs.device);
		DebugNameable<VK_OBJECT_TYPE_SEMAPHORE>::debugname_swap(rhs);
		return *this;
	}

	const LogicalDevice& BinarySemaphore::get_device() const
	{
		hdk::assert(this->device != nullptr && !this->device->is_null(), "BinarySemaphore had nullptr or null LogicalDevice");
		return *this->device;
	}

	BinarySemaphore::NativeType BinarySemaphore::native() const
	{
		return this->sem;
	}

	BinarySemaphore::BinarySemaphore():
	sem(VK_NULL_HANDLE),
	device(nullptr){}

	TimelineSemaphore::TimelineSemaphore(const LogicalDevice& device, std::uint64_t value):
	BinarySemaphore()
	{
		this->device = &device;
		hdk::assert(TimelineSemaphore::supported(device), "TimelineSemaphores are not supported by the LogicalDevice, but tried to create one. Please submit a bug report.");
		VkSemaphoreTypeCreateInfo create_type
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
			.pNext = nullptr,
			.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
			.initialValue = value
		};

		VkSemaphoreCreateInfo create
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = &create_type,
			.flags = 0
		};

		VkResult res = vkCreateSemaphore(this->get_device().native(), &create, nullptr, &this->sem);
		switch(res)
		{
			case VK_SUCCESS:
				// do nothing
			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				hdk::error("Failed to create TimelineSemaphore because we ran out of host memory (RAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				hdk::error("Failed to create TimelineSemaphore because we ran out of device memory (VRAM). Please ensure that your system meets the minimum requirements.");
			break;
			default:
				hdk::error("Failed to create TimelineSemaphore but cannot determine why. Please submit a bug report.");
			break;
		}
	}
	
	TimelineSemaphore::TimelineSemaphore(TimelineSemaphore&& move)
	{
		*this = std::move(move);
	}

	TimelineSemaphore::~TimelineSemaphore()
	{
		BinarySemaphore::~BinarySemaphore();
	}

	TimelineSemaphore& TimelineSemaphore::operator=(TimelineSemaphore&& rhs)
	{
		std::swap(this->sem, rhs.sem);
		std::swap(this->device, rhs.device);
		return *this;
	}

	void TimelineSemaphore::signal(std::uint64_t value)
	{
		VkSemaphoreSignalInfo signal
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
			.pNext = nullptr,
			.semaphore = this->native(),
			.value = value
		};
		vkSignalSemaphore(this->get_device().native(), &signal);
	}

	void TimelineSemaphore::wait_for(std::uint64_t value) const
	{
		VkSemaphoreWaitInfo wait
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
			.pNext = nullptr,
			.flags = 0,
			.semaphoreCount = 1,
			.pSemaphores = &this->sem,
			.pValues = &value
		};
		VkResult res = vkWaitSemaphores(this->get_device().native(), &wait, std::numeric_limits<std::uint64_t>::max());
		switch(res)
		{
			case VK_SUCCESS:
			[[fallthrough]];
			case VK_TIMEOUT:
			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				hdk::error("Failed to wait on TimelineSemaphore because we ran out of host memory (RAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				hdk::error("Failed to wait on TimelineSemaphore because we ran out of device memory (VRAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_DEVICE_LOST:
				hdk::error("Failed to wait on TimelineSemaphore because Device was lost. This is a fatal error.");
			break;
			default:
				hdk::error("Failed to wait on TimelineSemaphore but cannot determine why. Please submit a bug report.");
			break;
		}
	}
	
	bool TimelineSemaphore::supported(const LogicalDevice& device)
	{
		return device.get_features().contains(DeviceFeature::TimelineSemaphores);
	}

}

#endif // TZ_VULKAN
