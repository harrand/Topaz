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
		TimelineSemaphores,
		/// - Enables use of alpha blending.
		ColourBlendLogicalOperations
	};

	using DeviceFeatureField = tz::EnumField<DeviceFeature>;

	namespace detail
	{
		constexpr VkPhysicalDeviceDescriptorIndexingFeatures empty_descriptor_indexing_features()
		{
			VkPhysicalDeviceDescriptorIndexingFeatures feats{};
			feats.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
			feats.pNext = nullptr;
			return feats;
		}
	
		constexpr VkPhysicalDeviceVulkan12Features empty_12_features(VkPhysicalDeviceDescriptorIndexingFeatures& next)
		{
			VkPhysicalDeviceVulkan12Features feats{};
			feats.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
			feats.pNext = &next;
			return feats;
		}

		constexpr VkPhysicalDeviceFeatures2 empty_features2(VkPhysicalDeviceVulkan12Features& next)
		{
			VkPhysicalDeviceFeatures2 feats{};
			feats.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			feats.pNext = &next;
			return feats;
		}

		struct DeviceFeatureInfo
		{
			VkPhysicalDeviceDescriptorIndexingFeatures descriptor_indexing_features = empty_descriptor_indexing_features();
			VkPhysicalDeviceVulkan12Features features12 = empty_12_features(descriptor_indexing_features);
			VkPhysicalDeviceFeatures2 features = empty_features2(features12);
		};
	}
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_FEATURES_HPP
