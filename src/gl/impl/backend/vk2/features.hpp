#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_FEATURES_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_FEATURES_HPP
#if TZ_VULKAN
#include "gl/impl/backend/vk2/extensions.hpp"

namespace tz::gl::vk2
{
	/**
	 * @ingroup tz_gl_vk_extension
	 * @brief Represents a PhysicalDevice feature directly supported by Topaz.
	 * These may contain some of the Vulkan VkPhysicalDeviceFeatures, but it not limited to Vulkan-API specific stuff. Some of these features may be Topaz-centric.
	 */
	enum class PhysicalDeviceFeature
	{
		///  - Allows mass-batching of draw-calls. Vastly improves performance for large scenes.
		MultiDrawIndirect,

		BindlessDescriptors
	};

	using PhysicalDeviceFeatureField = tz::EnumField<PhysicalDeviceFeature>;

	namespace detail
	{
		struct PhysicalDeviceFeatureInfo
		{
			VkPhysicalDeviceDescriptorIndexingFeatures descriptor_indexing_features =
			{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
				.pNext = nullptr
			};
			VkPhysicalDeviceFeatures2 features =
			{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
				.pNext = &descriptor_indexing_features
			};
		};
	}
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_FEATURES_HPP
