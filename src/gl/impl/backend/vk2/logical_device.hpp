#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_LOGICAL_DEVICE_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_LOGICAL_DEVICE_HPP
#if TZ_VULKAN
#include "gl/impl/backend/vk2/hardware/physical_device.hpp"

namespace tz::gl::vk2
{
	/**
	 * @ingroup tz_gl_vk
	 * @{
	 */

	/**
	 * @brief PhysicalDevices contain families of queues. Each family can fulfill a variety of different purposes, such as graphics or compute. This enum lists all such usages.
	 * @note Queue Families may have more than one type, and due to this are associated with a @ref QueueFamilyTypeField.
	 */
	enum class QueueFamilyType
	{
		/// - Indicates that queues within this family are suitable for graphics operations.
		Graphics,
		/// - Indicates that queues within this family are suitable for compute operations.
		Compute,
		/// - Indicates that queues within this family are suitable for transfer operations.
		Transfer
	};

	/**
	 * An @ref EnumField of QueueFamilyType. Can wholly describe the usage of one of a PhysicalDevice's queue families.
	 */
	using QueueFamilyTypeField = tz::EnumField<QueueFamilyType>;

	/**
	 * @}
	 */

	struct QueueFamilyInfo
	{
		std::uint32_t family_size;
		bool present_support;
		QueueFamilyTypeField types;
	};

	struct LogicalDeviceInfo
	{
		PhysicalDevice physical_device = {};
		DeviceExtensionList extensions = {};
		PhysicalDeviceFeatureField features = {};
		const WindowSurface* surface = nullptr;
	};
	
	/**
	 * @ingroup tz_gl_vk
	 * @brief Logical interface to an existing @ref PhysicalDevice
	 */
	class LogicalDevice
	{
	public:
		/**
		 * @brief Construct a LogicalDevice based on a PhysicalDevice, and some optional extensions/features to enable.
		 * @pre All elements of `enabled_extensions` are supported. That is, are contained within @ref PhysicalDevice::get_supported_extensions and @ref PhysicalDevice::get_supported_features. If an extension/feature is enabled which is not supported by `physical_device`, the behaviour is undefined.
		 */
		LogicalDevice(LogicalDeviceInfo device_info);

		/**
		 * @brief Retrieve the PhysicalDevice that this LogicalDevice derives from.
		 */
		const PhysicalDevice& get_hardware() const;
		/**
		 * @brief Retrieve a list of all enabled extensions.
		 */
		const DeviceExtensionList& get_extensions() const;
		VkDevice native() const;
	private:
		VkDevice dev;
		PhysicalDevice physical_device;
		DeviceExtensionList enabled_extensions;
		PhysicalDeviceFeatureField enabled_features;
		std::vector<QueueFamilyInfo> queue_families;
	};
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_LOGICAL_DEVICE_HPP
