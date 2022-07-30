#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_FENCE_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_FENCE_HPP
#if TZ_VULKAN
#include "tz/gl/impl/backend/vk2/logical_device.hpp"
#include "tz/gl/impl/backend/vk2/debugname.hpp"

namespace tz::gl::vk2
{
	/**
	 * @ingroup tz_gl_vk_sync
	 * Specifies creation flags for a @ref Fence.
	 */
	struct FenceInfo
	{
		/// Owning @ref LogicalDevice. Must not be null.
		const LogicalDevice* device;
		/// True if the @ref Fence should initially be signalled, otherwise false. By default, this is false.
		bool initially_signalled = false;
	};

	/**
	 * @ingroup tz_gl_vk_sync
	 * Synchronisation primitive which is useful to detect completion of a GPU operation, CPU-side.
	 */
	class Fence : public DebugNameable<VK_OBJECT_TYPE_FENCE>
	{
	public:
		Fence(FenceInfo info);
		Fence(const Fence& copy) = delete;
		Fence(Fence&& move);
		~Fence();

		Fence& operator=(const Fence& rhs) = delete;
		Fence& operator=(Fence&& rhs);

		const LogicalDevice& get_device() const;
		/**
		 * Query as to whether the Fence is currently signalled.
		 * @return True if signalled, otherwise false.
		 */
		bool is_signalled() const;
		/**
		 * Block the current thread until the Fence is signalled.
		 * @post `this->is_signalled()` returns true.
		 */
		void wait_until_signalled() const;
		/**
		 * Set the state of the Fence to the unsignaleld state.
		 */
		void unsignal();

		using NativeType = VkFence;
		NativeType native() const;
	private:
		VkFence fence;
		FenceInfo info;
	};
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_FENCE_HPP
