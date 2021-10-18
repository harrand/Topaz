#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_HARDWARE_PHYSICAL_DEVICE_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_HARDWARE_PHYSICAL_DEVICE_HPP
#if TZ_VULKAN
#include "core/containers/basic_list.hpp"
#include "core/containers/enum_field.hpp"
#include "vulkan/vulkan.h"

namespace tz::gl::vk2
{
	enum class PhysicalDeviceFeature
	{
		MultiDrawIndirect
	};

	using PhysicalDeviceFeatureField = tz::EnumField<PhysicalDeviceFeature>;

	namespace detail
	{
		PhysicalDeviceFeatureField to_feature_field(VkPhysicalDeviceFeatures features);
	}

	class PhysicalDevice
	{
	public:
		PhysicalDevice(VkPhysicalDevice native);

		PhysicalDeviceFeatureField get_supported_features() const;
	private:
		VkPhysicalDevice dev;
		
	};

	using PhysicalDeviceList = tz::BasicList<PhysicalDevice>;

	PhysicalDeviceList get_all_devices();
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_HARDWARE_PHYSICAL_DEVICE_HPP
