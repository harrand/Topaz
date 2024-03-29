#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_FEATURES_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_FEATURES_HPP
#if TZ_VULKAN
#include "tz/gl/impl/vulkan/detail/extensions.hpp"

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
		/// - Allows GPU-driven rendering via draw-indirect-count. Can be used to minimise cpu-gpu latency.
		DrawIndirectCount,
		/// - Allows mass-batching of draw-calls. Vastly improves performance for large scenes.
		MultiDrawIndirect,
		/// - Enable shader draw parameters (gl_DrawID).
		ShaderDrawParameters,
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
		FragmentShaderResourceWrite,
		/// - Enables dynamic rendering. this is not optional.
		DynamicRendering
	};

	using DeviceFeatureField = tz::enum_field<DeviceFeature>;

	namespace detail
	{
		constexpr VkPhysicalDeviceVulkan11Features empty_11_features(VkPhysicalDeviceVulkan12Features& next)
		{
			VkPhysicalDeviceVulkan11Features feats{};
			feats.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
			feats.pNext = &next;
			return feats;
		}

		constexpr VkPhysicalDeviceVulkan12Features empty_12_features(VkPhysicalDeviceVulkan13Features& next)
		{
			VkPhysicalDeviceVulkan12Features feats{};
			feats.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
			feats.pNext = &next;	
			return feats;
		}

		constexpr VkPhysicalDeviceVulkan13Features empty_13_features()
		{
			VkPhysicalDeviceVulkan13Features feats{};
			feats.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
			feats.pNext = nullptr;
			return feats;
		}

		constexpr VkPhysicalDeviceFeatures2 empty_features2(VkPhysicalDeviceVulkan11Features& next)
		{
			VkPhysicalDeviceFeatures2 feats{};
			feats.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			feats.pNext = &next;
			return feats;
		}

		struct DeviceFeatureInfo
		{
			VkPhysicalDeviceVulkan13Features features13 = empty_13_features();
			VkPhysicalDeviceVulkan12Features features12 = empty_12_features(features13);
			VkPhysicalDeviceVulkan11Features features11 = empty_11_features(features12);
			VkPhysicalDeviceFeatures2 features = empty_features2(features11);
		};
	}
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_FEATURES_HPP
