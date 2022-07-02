#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_BUFFER_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_BUFFER_HPP
#if TZ_VULKAN
#include "gl/impl/backend/vk2/logical_device.hpp"
#include "gl/impl/backend/vk2/gpu_mem.hpp"

namespace tz::gl::vk2
{
	/**
	 * @ingroup tz_gl_vk_buffer
	 * Specifies allowed usages for a @ref Buffer.
	 */
	enum class BufferUsage
	{
		/// - Can be used as the source of a transfer command.
		TransferSource = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		/// - Can be used as the destination of a transfer command.
		TransferDestination = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		/// - Is a UniformBuffer, which is a type of shader buffer resource.
		UniformBuffer = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		/// - Is a StorageBuffer, which is a type of shader buffer resource.
		StorageBuffer = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		/// - Is a specialised buffer which specifically stores draw commands.
		DrawIndirectBuffer = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,

		/// - Is a VertexBuffer, which is used to store vertex input data.
		VertexBuffer = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		/// - Is an IndexBuffer, which is used to store index input data.
		IndexBuffer = VK_BUFFER_USAGE_INDEX_BUFFER_BIT
	};

	using BufferUsageField = tz::EnumField<BufferUsage>;

	/**
	 * @ingroup tz_gl_vk_buffer
	 * Specifies creation flags for a @ref Buffer.
	 */
	struct BufferInfo
	{
		/// Owning device. Must not be nullptr or null.
		const LogicalDevice* device;
		/// Size of the buffer, in bytes.
		std::size_t size_bytes;
		/// Field containing all usages of the buffer.
		BufferUsageField usage;
		/// Describes where in memory buffer data will lie.
		MemoryResidency residency;
	};

	/**
	 * @ingroup tz_gl_vk_buffer
	 * Represents a linear array of data which can be used for various purposes. See @ref BufferUsage for some examples of usages.
	 */
	class Buffer
	{
	public:
		Buffer(BufferInfo info);
		Buffer(const Buffer& copy) = delete;
		Buffer(Buffer&& move);
		~Buffer();

		Buffer& operator=(const Buffer& rhs) = delete;
		Buffer& operator=(Buffer&& rhs);

		/**
		 * Retrieve the @ref LogicalDevice used to create the buffer.
		 */
		const LogicalDevice& get_device() const;
		/**
		 * Retrieve all usages of this buffer.
		 */
		BufferUsageField get_usage() const;
		/**
		 * Retrieve the residency of the buffer's memory.
		 */
		MemoryResidency get_residency() const;
		/**
		 * If possible, map the memory to a CPU-size pointer.
		 * @return Pointer to mapped memory, or nullptr if something went wrong.
		 */
		void* map();
		/**
		 * Map the memory to a span of a given type.
		 * @tparam T Type to map to. Must be TriviallyCopyable.
		 * @return Span of size @ref Buffer::size on success. The mapping can fail, in which case an invalid span of size 0 is returned.
		 */
		template<typename T>
		std::span<T> map_as()
		{
			auto* ptr = reinterpret_cast<T*>(this->map());
			if(ptr == nullptr)
			{
				return {ptr, 0};
			}
			return {ptr, this->size() / sizeof(T)};
		}
		/**
		 * Unmap the buffer, causing any pointers created from @ref Buffer::map to be invalidated.
		 */
		void unmap();
		/**
		 * Retrieve the size, in bytes, of the buffer.
		 * @return Size in bytes.
		 */
		std::size_t size() const;

		std::string debug_get_name() const;
		void debug_set_name(std::string name);

		static Buffer null();
		bool is_null() const;

		using NativeType = VkBuffer;
		NativeType native() const;
	private:
		Buffer();

		VkBuffer buffer;
		BufferInfo info;
		VmaAllocation vma_alloc;
		VmaAllocationInfo vma_alloc_info;
		std::string debug_name = "";
	};
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_BUFFER_HPP
