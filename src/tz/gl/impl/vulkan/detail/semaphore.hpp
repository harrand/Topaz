#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_SEMAPHORE_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_SEMAPHORE_HPP
#if TZ_VULKAN
#include "tz/gl/impl/vulkan/detail/logical_device.hpp"
#include "tz/gl/impl/vulkan/detail/debugname.hpp"

namespace tz::gl::vk2
{
	enum class semaphore_type
	{
		binary,
		timeline
	};
	/**
	 * @ingroup tz_gl_vk_sync
	 * Synchronisation primitive which is not interactable on the host and which has two states:
	 * - Signalled
	 * - Unsignalled
	 * Useful for the GPU detecting the completion of another GPU operation.
	 */
	class BinarySemaphore : public DebugNameable<VK_OBJECT_TYPE_SEMAPHORE>
	{
	public:
		BinarySemaphore(const LogicalDevice& device);
		BinarySemaphore(const BinarySemaphore& copy) = delete;
		BinarySemaphore(BinarySemaphore&& move);
		virtual ~BinarySemaphore();

		BinarySemaphore& operator=(const BinarySemaphore& rhs) = delete;
		BinarySemaphore& operator=(BinarySemaphore&& rhs);
		virtual semaphore_type get_type() const{return semaphore_type::binary;}

		const LogicalDevice& get_device() const;

		using NativeType = VkSemaphore;
		NativeType native() const;

		static BinarySemaphore null();
		bool is_null() const;
	protected:
		BinarySemaphore();
		VkSemaphore sem;
		const LogicalDevice* device;
	};

	/**
	 * @ingroup tz_gl_vk_sync
	 * Alias for @ref BinarySemaphore
	 */
	using Semaphore = BinarySemaphore;

	/**
	 * @ingroup tz_gl_vk_sync
	 * Synchronisation primitive similar to @ref BinarySemaphore. Semaphore with a strictly increasing 64-bit unsigned integer payload. They are signalled with respect to a particular reference value. Note that this is an optional feature.
	 *
	 * In addition, TimelineSemaphores can be signalled/waited-on directly by the host. This makes them the most host-interactable synchronisation primitive, more than the @ref BinarySemaphore and even more than the @ref Fence.
	 */
	class TimelineSemaphore : public BinarySemaphore
	{
	public:
		/**
		 * Create a TimelineSemaphore with the initial value.
		 * @pre `device.get_features()` contains @ref DeviceFeature::TimelineSemaphores
		 */
		TimelineSemaphore(const LogicalDevice& device, std::uint64_t value = 0);
		TimelineSemaphore(const TimelineSemaphore& copy) = delete;
		TimelineSemaphore(TimelineSemaphore&& move);
		~TimelineSemaphore();
		TimelineSemaphore& operator=(const TimelineSemaphore&& rhs) = delete;
		TimelineSemaphore& operator=(TimelineSemaphore&& rhs);


		virtual semaphore_type get_type() const{return semaphore_type::timeline;}
		/**
		 * Instantaneously set the semaphore to the given value.
		 */
		void signal(std::uint64_t value);
		/**
		 * Blocks the current thread and waits until the semaphore has reached the provided value.
		 */
		void wait_for(std::uint64_t value) const;
		/**
		 * Retrieve the current semaphore value.
		 */
		std::uint64_t get_value() const;

		/**
		 * Timeline Semaphores are optional features and must be enabled. This is a helper function which can determine if a @ref LogicalDevice has enabled Timeline Semaphores.
		 *
		 * See @ref DeviceFeature::TimelineSemaphores for context.
		 */
		static bool supported(const LogicalDevice& device);
	};


}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_SEMAPHORE_HPP
