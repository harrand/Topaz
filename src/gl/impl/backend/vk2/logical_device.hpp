#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_LOGICAL_DEVICE_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_LOGICAL_DEVICE_HPP
#if TZ_VULKAN
#include "gl/impl/backend/vk2/hardware/physical_device.hpp"

namespace tz::gl::vk2
{
	enum class QueueFamilyType
	{
		Graphics,
		Compute,
		Transfer
	};
	using QueueFamilyTypeField = tz::EnumField<QueueFamilyType>;

	struct QueueFamilyInfo
	{
		std::uint32_t family_size;
		QueueFamilyTypeField types;
	};
	/**
	 * Logical interface to an existing PhysicalDevice
	 */
	class LogicalDevice
	{
	public:
		LogicalDevice(PhysicalDevice physical_device, ExtensionList enabled_extensions = {}, PhysicalDeviceFeatureField enabled_features = {});

		const PhysicalDevice& get_hardware() const;
		const ExtensionList& get_extensions() const;
		VkDevice native() const;
	private:
		VkDevice dev;
		PhysicalDevice physical_device;
		ExtensionList enabled_extensions;
		PhysicalDeviceFeatureField enabled_features;
	};
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_LOGICAL_DEVICE_HPP
