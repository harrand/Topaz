#if TZ_VULKAN
#include "gl/impl/backend/vk2/buffer.hpp"

namespace tz::gl::vk2
{
	Buffer::Buffer(BufferInfo info):
	buffer(VK_NULL_HANDLE),
	info(info),
	vma_alloc(),
	vma_alloc_info()
	{
		VkBufferCreateInfo create
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.size = static_cast<VkDeviceSize>(this->info.size_bytes),
			.usage = static_cast<VkBufferUsageFlags>(static_cast<BufferUsage>(info.usage)),
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = nullptr
		};

		VmaMemoryUsage vma_usage;
		VmaAllocationCreateFlags vma_flags = 0;
		switch(info.residency)
		{
			case MemoryResidency::GPU:
				vma_usage = VMA_MEMORY_USAGE_GPU_ONLY;
			break;
			case MemoryResidency::CPU:
				vma_usage = VMA_MEMORY_USAGE_CPU_ONLY;
			break;
			case MemoryResidency::CPUPersistent:
				vma_usage = VMA_MEMORY_USAGE_CPU_ONLY;
				vma_flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
			break;
			default:
				tz_error("Unrecognised MemoryResidency. Please submit a bug report.");
			break;
		}

		VmaAllocationCreateInfo alloc
		{
			.flags = vma_flags,
			.usage = vma_usage,
			.requiredFlags = 0,
			.preferredFlags = 0,
			.memoryTypeBits = 0,
			.pool = {},
			.pUserData = nullptr,
			.priority = 0
		};

		VkResult res = vmaCreateBuffer(this->get_device().vma_native(), &create, &alloc, &this->buffer, &this->vma_alloc, &this->vma_alloc_info);
		switch(res)
		{
			case VK_SUCCESS:

			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				tz_error("Failed to create Buffer because we ran out of host memory (RAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				tz_error("Failed to create Buffer because we ran out of device memory (VRAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
				tz_error("Failed to create Buffer because the requested address is not available. Note: This error type should never happen, please submit a bug report.");
			break;
			default:
				tz_error("Failed to create Buffer but cannot determine why. Please submit a bug report.");
			break;
		}
	}

	Buffer::Buffer(Buffer&& move):
	buffer(VK_NULL_HANDLE),
	info(),
	vma_alloc(),
	vma_alloc_info()
	{
		*this = std::move(move);
	}

	Buffer::~Buffer()
	{
		if(this->buffer != VK_NULL_HANDLE)
		{
			vmaDestroyBuffer(this->get_device().vma_native(), this->buffer, this->vma_alloc);
			this->buffer = VK_NULL_HANDLE;
		}
	}

	Buffer& Buffer::operator=(Buffer&& rhs)
	{
		std::swap(this->buffer, rhs.buffer);
		std::swap(this->info, rhs.info);
		std::swap(this->vma_alloc, rhs.vma_alloc);
		std::swap(this->vma_alloc_info, rhs.vma_alloc_info);
		return *this;
	}

	const LogicalDevice& Buffer::get_device() const
	{
		tz_assert(this->info.device != nullptr && !this->info.device->is_null(), "BufferInfo contained nullptr or null LogicalDevice");
		return *this->info.device;
	}

	BufferUsageField Buffer::get_usage() const
	{
		return this->info.usage;
	}

	void* Buffer::map()
	{
		// If we're GPU resident we can't be host visible, so early out.
		if(this->info.residency == MemoryResidency::GPU)
		{
			return nullptr;
		}
		// If we've mapped earlier/are CPUPersistent, we already have a ptr we can use.
		if(this->vma_alloc_info.pMappedData != nullptr)
		{
			return this->vma_alloc_info.pMappedData;
		}
		// Otherwise we'll actually perform the map.
		VkResult res = vmaMapMemory(this->get_device().vma_native(), this->vma_alloc, &this->vma_alloc_info.pMappedData);
		if(res == VK_SUCCESS)
		{
			return this->vma_alloc_info.pMappedData;
		}
		return nullptr;
	}

	void Buffer::unmap()
	{
		if(this->vma_alloc_info.pMappedData != nullptr)
		{
			return;
		}

		vmaUnmapMemory(this->get_device().vma_native(), this->vma_alloc);
		this->vma_alloc_info.pMappedData = nullptr;
	}

	std::size_t Buffer::size() const
	{
		return this->info.size_bytes;
	}

	Buffer::NativeType Buffer::native() const
	{
		return this->buffer;
	}
}

#endif // TZ_VULKAN
