#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_HARDWARE_PHYSICAL_DEVICE_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_HARDWARE_PHYSICAL_DEVICE_HPP
#if TZ_VULKAN
#include "core/containers/basic_list.hpp"
#include "core/containers/enum_field.hpp"
#include "vulkan/vulkan.h"

namespace tz::gl::vk2
{
	// Represents a Vulkan feature directly supported by Topaz.
	enum class PhysicalDeviceFeature
	{
		MultiDrawIndirect
	};

	using PhysicalDeviceFeatureField = tz::EnumField<PhysicalDeviceFeature>;

	namespace detail
	{
		/**
		 * Convert a supported Vulkan feature into its underlying representation for the Vulkan API.
		 */
		PhysicalDeviceFeatureField to_feature_field(VkPhysicalDeviceFeatures features);
	}

	/**
	 * Represents something resembling a graphics card that can perform general graphical operations. A PhysicalDevice may or may not support graphics, compute or transfer work.
	 */
	class PhysicalDevice
	{
	public:
		/**
		 * Retrieve a PhysicalDevice from a Vulkan API handle.
		 * @param native Vulkan API Handle.
		 * Note: You almost certainly don't want to do this. See @ref get_all_devices to retrieve the physical devices available at runtime.
		 */
		PhysicalDevice(VkPhysicalDevice native);
		/**
		 * PhysicalDevices do not necessarily support all available PhysicalDeviceFeatures.
		 * @return An EnumField containing all the features supported by this Physical Device.
		 */
		PhysicalDeviceFeatureField get_supported_features() const;
		/**
		 * Retrieve the native Vulkan API Handle corresponding to this PhysicalDevice.
		 */
		VkPhysicalDevice native() const;
	private:
		VkPhysicalDevice dev;
	};

	using PhysicalDeviceList = tz::BasicList<PhysicalDevice>;

	/**
	 * Retrieve a list of all physical devices available on the machine
	 * @return BasicList of all PhysicalDevices. These have not been filtered in any way.
	 */
	PhysicalDeviceList get_all_devices();
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_HARDWARE_PHYSICAL_DEVICE_HPP
