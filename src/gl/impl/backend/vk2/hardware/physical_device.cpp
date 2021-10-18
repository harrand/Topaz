#if TZ_VULKAN
#include "gl/impl/backend/vk2/hardware/physical_device.hpp"
#include "gl/impl/backend/vk2/tz_vulkan.hpp"
#include <vector>

namespace tz::gl::vk2
{
	namespace detail
	{
		PhysicalDeviceFeatureField to_feature_field(VkPhysicalDeviceFeatures features)
		{
			PhysicalDeviceFeatureField ret;

			if(features.multiDrawIndirect)
			{
				ret |= PhysicalDeviceFeature::MultiDrawIndirect;
			}
		
			return ret;
		}
	}

	PhysicalDevice::PhysicalDevice(VkPhysicalDevice native):
	dev(native){}

	PhysicalDeviceFeatureField PhysicalDevice::get_supported_features() const
	{
		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceFeatures(this->dev, &features);

		return detail::to_feature_field(features);
	}

	PhysicalDeviceList get_all_devices()
	{
		PhysicalDeviceList devices;
		std::vector<VkPhysicalDevice> device_natives;

		std::uint32_t physical_device_count;
		vkEnumeratePhysicalDevices(vk2::get().native(), &physical_device_count, nullptr);

		device_natives.resize(static_cast<std::size_t>(physical_device_count));
		for(VkPhysicalDevice device_native : device_natives)
		{
			devices.emplace(device_native);
		}
		return devices;
	}
}

#endif // TZ_VULKAN
