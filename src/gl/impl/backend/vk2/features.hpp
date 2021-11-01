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
		MultiDrawIndirect ///  - Allows mass-batching of draw-calls. Vastly improves performance for large scenes.
	};

	using PhysicalDeviceFeatureField = tz::EnumField<PhysicalDeviceFeature>;
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_FEATURES_HPP
