#ifndef TOPAZ_GL_VK_BUFFER_HPP
#define TOPAZ_GL_VK_BUFFER_HPP
#if TZ_VULKAN
#include "gl/impl/backend/vk/logical_device.hpp"
#include <cstdint>

namespace tz::gl::vk
{
	enum class BufferType
	{
		Vertex,
		Index,
		Staging,
		Uniform,
		ShaderStorage,
		DrawIndirect
	};

	enum class BufferPurpose
	{
		TransferSource,
		TransferDestination,
		NothingSpecial
	};

	class Buffer
	{
	public:
		Buffer(BufferType type, BufferPurpose purpose, const LogicalDevice& device, hardware::MemoryResidency residency, std::size_t bytes);
		Buffer(const Buffer& copy) = delete;
		Buffer(Buffer&& move);
		~Buffer();
		Buffer& operator=(const Buffer& rhs) = delete;
		Buffer& operator=(Buffer&& rhs);

		BufferType get_type() const;

		void write(const void* addr, std::size_t bytes);
		void* map_memory();
		void unmap_memory();

		VkBuffer native() const;
		bool is_null() const;
		static Buffer null();
	private:
		Buffer();
		void ensure_notnull() const;

		VkBuffer buffer;
		void* persistent_mapped_ptr;
		VmaAllocation alloc;
		const LogicalDevice* device;
		BufferType type;
	};
}

#endif // TZ_VULKAN
#endif //TOPAZ_GL_VK_BUFFER_HPP