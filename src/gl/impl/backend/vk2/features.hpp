#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_FEATURES_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_FEATURES_HPP
#if TZ_VULKAN
#include "gl/impl/backend/vk2/extensions.hpp"

namespace tz::gl::vk2
{
	/**
	 * @ingroup tz_gl_vk_extension
	 * @brief Represents an optional feature specific to a @ref PhysicalDevice.
	 *
	 * - DeviceFeatures provide various advantages/additional features.
	 * - No features are enabled by default.
	 * - DeviceFeatures may not be supported by a PhysicalDevice. See @ref PhysicalDevice::get_supported_features to retrieve a list of supported features.
	 */
	enum class DeviceFeature
	{
		/// - Allows mass-batching of draw-calls. Vastly improves performance for large scenes.
		MultiDrawIndirect,
		/// - Allows creation of bindless descriptors. Dramatically increases the limits for shader resources, and yields a moderate to vast improvement to performance across-the-board.
		BindlessDescriptors,
		/// - Enables use of @ref TimelineSemaphore.
		TimelineSemaphores
	};

	using DeviceFeatureField = tz::EnumField<DeviceFeature>;

	namespace detail
	{
		struct DeviceFeatureInfo
		{
			VkPhysicalDeviceDescriptorIndexingFeatures descriptor_indexing_features =
			{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
				.pNext = nullptr
			};
			VkPhysicalDeviceVulkan12Features features12 =
			{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
				.pNext = &descriptor_indexing_features
			};
			VkPhysicalDeviceFeatures2 features =
			{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
				.pNext = &features12
			};
		};
	}
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_FEATURES_HPP
