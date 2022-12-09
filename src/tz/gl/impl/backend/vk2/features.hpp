#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_FEATURES_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_FEATURES_HPP
#if TZ_VULKAN
#include "tz/gl/impl/backend/vk2/extensions.hpp"

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
		ColourBlendLogicalOperations,
		/// - Enables use of rasteriser polygon modes which aren't filled, e.g wireframe.
		NonSolidFillRasteriser,
		/// - Enables use of tesselation-control and tessellation-evaluation shaders.
		TessellationShaders,
		/// - Enables vertex, geometry, and tessellation shaders to write to storage buffers.
		VertexPipelineResourceWrite,
		/// - Enables fragment shaders to write to storage buffers.
		FragmentShaderResourceWrite
	};

	using DeviceFeatureField = tz::EnumField<DeviceFeature>;

	namespace detail
	{
		constexpr VkPhysicalDeviceVulkan12Features empty_12_features()
		{
			VkPhysicalDeviceVulkan12Features feats{};
			feats.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
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
			VkPhysicalDeviceVulkan12Features features12 = empty_12_features();
			VkPhysicalDeviceFeatures2 features = empty_features2(features12);
		};
	}
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_FEATURES_HPP
