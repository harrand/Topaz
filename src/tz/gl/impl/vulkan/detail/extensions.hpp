#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_EXTENSIONS_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_EXTENSIONS_HPP
#include "tz/core/data/basic_list.hpp"
#include "tz/core/data/enum_field.hpp"
#include "vulkan/vulkan.h"
#include <array>
#include <cstring>

namespace tz::gl::vk2
{
	/**
	 * @ingroup tz_gl_vk_extension
	 * @{
	 */

	/**
	 * Supported Vulkan extension for a VulkanInstance.
	 */
	enum class InstanceExtension
	{
		/// - Enables Debug Messenger (only supported on TZ_DEBUG)
		DebugMessenger,

		Count
	};

	/**
	 * Supported Vulkan extension for a LogicalDevice.
	 */
	enum class DeviceExtension
	{
		/// - Enables Swapchain operations (required to create a swapchain and present images)
		Swapchain,
		ShaderDebugPrint,
		/// - Needed for Tracy profiling
		CalibratedTimestamps,

		Count
	};

	using InstanceExtensionList = tz::enum_field<InstanceExtension>;
	using DeviceExtensionList = tz::enum_field<DeviceExtension>;

	/**
	 * @}
	 */

	namespace util
	{
		constexpr std::array<const char*, static_cast<int>(InstanceExtension::Count)> instance_extension_tz_names{"Debug Messenger"};
		constexpr std::array<const char*, static_cast<int>(DeviceExtension::Count)> device_extension_tz_names{"Swapchain", "Shader Debug Print", "Calibrated Timestamps"};

		using VkExtension = const char*;
		constexpr std::array<VkExtension, static_cast<int>(InstanceExtension::Count)> instance_extension_names{VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
		constexpr std::array<VkExtension, static_cast<int>(DeviceExtension::Count)> device_extension_names{VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME, VK_EXT_CALIBRATED_TIMESTAMPS_EXTENSION_NAME};

		constexpr VkExtension to_vk_extension(InstanceExtension extension)
		{
			using SizeType = decltype(instance_extension_names)::size_type;
			return instance_extension_names[static_cast<SizeType>(static_cast<int>(extension))];
		}

		constexpr VkExtension to_vk_extension(DeviceExtension extension)
		{
			using SizeType = decltype(device_extension_names)::size_type;
			return device_extension_names[static_cast<SizeType>(static_cast<int>(extension))];
		}

		constexpr InstanceExtension to_instance_extension(VkExtension extension)
		{
			auto iter = std::find_if(instance_extension_names.begin(), instance_extension_names.end(), [extension](VkExtension cur_ext)->bool{return std::strcmp(cur_ext, extension) == 0;});
			if(iter != instance_extension_names.end())
			{
				return static_cast<InstanceExtension>(static_cast<int>(std::distance(instance_extension_names.begin(), iter)));
			}
			return InstanceExtension::Count;
		}
		
		constexpr DeviceExtension to_device_extension(VkExtension extension)
		{
			auto iter = std::find_if(device_extension_names.begin(), device_extension_names.end(), [extension](VkExtension cur_ext)->bool{return std::strcmp(cur_ext, extension) == 0;});
			if(iter != device_extension_names.end())
			{
				return static_cast<DeviceExtension>(static_cast<int>(std::distance(device_extension_names.begin(), iter)));
			}
			return DeviceExtension::Count;
		}

		using VkExtensionList = tz::basic_list<VkExtension>;
	}
}

#endif // TOPAZ_GL_IMPL_BACKEND_VK2_EXTENSIONS_HPP
