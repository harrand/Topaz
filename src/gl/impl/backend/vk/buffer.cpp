#if TZ_VULKAN
#include "gl/impl/backend/vk/buffer.hpp"
#include <cstring>

namespace tz::gl::vk
{
	Buffer::Buffer(BufferType type, BufferPurpose purpose, const LogicalDevice& device, hardware::MemoryResidency residency, std::size_t size_bytes):
	buffer(VK_NULL_HANDLE),
	persistent_mapped_ptr(nullptr),
	alloc(),
	device(&device),
	type(type)
	{
		VkBufferCreateInfo create{};
		create.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		create.size = static_cast<VkDeviceSize>(size_bytes);
		switch(type)
		{
			case BufferType::Vertex:
				create.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			break;
			case BufferType::Index:
				create.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			break;
			case BufferType::Staging:

			break;
			case BufferType::Uniform:
				create.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			break;
			case BufferType::ShaderStorage:
				create.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			break;
			case BufferType::DrawIndirect:
				create.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
			break;
			default:
				tz_error("Unrecognised BufferType");
			break;
		}
		switch(purpose)
		{
			case BufferPurpose::TransferSource:
				tz_assert(type == BufferType::Staging, "Buffer with TransferSource purpose (Staging Buffer) is incompatible with all BufferTypes aside from Staging");
				create.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			break;
			case BufferPurpose::TransferDestination:
				// Any buffer can be a transfer destination
				create.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			break;
			case BufferPurpose::NothingSpecial:

			break;
			default:
				tz_error("Unrecognised BufferPurpose");
			break;
		}
		create.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo alloc_info{};
		switch(residency)
		{
			case hardware::MemoryResidency::CPUPersistent:
				alloc_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
				[[fallthrough]];
			case hardware::MemoryResidency::CPU:
				alloc_info.usage = VMA_MEMORY_USAGE_CPU_ONLY;
			break;
			case hardware::MemoryResidency::GPU:
				alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
			break;
			default:
				tz_error("Unexpected MemoryResidency");
			break;
		}

		VmaAllocationInfo alloc_result;
		auto res = vmaCreateBuffer(this->device->native_allocator(), &create, &alloc_info, &this->buffer, &this->alloc, &alloc_result);
		if(residency == hardware::MemoryResidency::CPUPersistent)
		{
			this->persistent_mapped_ptr = alloc_result.pMappedData;
		}
		tz_assert(res == VK_SUCCESS, "Failed to create buffer");
	}

	Buffer::Buffer(Buffer&& move):
	buffer(VK_NULL_HANDLE),
	persistent_mapped_ptr(nullptr),
	alloc(),
	device(nullptr),
	type()
	{
		*this = std::move(move);
	}

	Buffer::~Buffer()
	{
		if(this->buffer != VK_NULL_HANDLE)
		{
			vmaDestroyBuffer(this->device->native_allocator(), this->buffer, this->alloc);
			this->buffer = VK_NULL_HANDLE;
		}
	}

	BufferType Buffer::get_type() const
	{
		this->ensure_notnull();
		return this->type;
	}

	void Buffer::write(const void* addr, std::size_t bytes)
	{
		this->ensure_notnull();
		void* data = this->map_memory();
		std::memcpy(data, addr, bytes);
		this->unmap_memory();
	}

	void* Buffer::map_memory()
	{
		this->ensure_notnull();
		if(this->persistent_mapped_ptr != nullptr)
		{
			return this->persistent_mapped_ptr;
		}
		void* data;
		vmaMapMemory(this->device->native_allocator(), this->alloc, &data);
		return data;
	}

	void Buffer::unmap_memory()
	{
		this->ensure_notnull();
		if(this->persistent_mapped_ptr != nullptr)
		{
			return;
		}
		vmaUnmapMemory(this->device->native_allocator(), this->alloc);
	}

	Buffer& Buffer::operator=(Buffer&& rhs)
	{
		std::swap(this->buffer, rhs.buffer);
		std::swap(this->persistent_mapped_ptr, rhs.persistent_mapped_ptr);
		std::swap(this->alloc, rhs.alloc);
		std::swap(this->device, rhs.device);
		std::swap(this->type, rhs.type);
		return *this;
	}

	VkBuffer Buffer::native() const
	{
		return this->buffer;
	}

	bool Buffer::is_null() const
	{
		return this->buffer == VK_NULL_HANDLE;
	}

	Buffer Buffer::null()
	{
		return Buffer{};
	}

	Buffer::Buffer():
	buffer(VK_NULL_HANDLE),
	persistent_mapped_ptr(nullptr),
	alloc(),
	device(nullptr),
	type()
	{}

	void Buffer::ensure_notnull() const
	{
		tz_assert(!this->is_null(), "Attempted to perform operation on null vk::Buffer");
	}
}

#endif // TZ_VULKAN