#include "gl/impl/backend/vk2/image_format.hpp"
#include "gl/impl/frontend/common/shader.hpp"
#if TZ_VULKAN
#include "gl/impl/backend/vk2/hardware/physical_device.hpp"
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

		VkPhysicalDeviceFeatures from_feature_field(const PhysicalDeviceFeatureField& feature_field)
		{
			VkPhysicalDeviceFeatures features{};
			features.multiDrawIndirect = feature_field.contains(PhysicalDeviceFeature::MultiDrawIndirect) ? VK_TRUE : VK_FALSE;
			return features;
		}

		PhysicalDeviceVendor to_tz_vendor(VkDriverId driver_id)
		{
			switch(driver_id)
			{
				case VK_DRIVER_ID_AMD_PROPRIETARY:
					[[fallthrough]];
				case VK_DRIVER_ID_AMD_OPEN_SOURCE:
					return PhysicalDeviceVendor::AMD;
				break;
				case VK_DRIVER_ID_NVIDIA_PROPRIETARY:
					return PhysicalDeviceVendor::Nvidia;
				break;
				case VK_DRIVER_ID_INTEL_PROPRIETARY_WINDOWS:
					[[fallthrough]];
				case VK_DRIVER_ID_INTEL_OPEN_SOURCE_MESA:
					return PhysicalDeviceVendor::Intel;
				break;
				default:
					return PhysicalDeviceVendor::Other;
				break;
			}
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

	DeviceExtensionList PhysicalDevice::get_supported_extensions() const
	{
		std::uint32_t supported_extension_count;
		vkEnumerateDeviceExtensionProperties(this->dev, nullptr, &supported_extension_count, nullptr);
		std::vector<VkExtensionProperties> props;
		std::vector<util::VkExtension> extensions; 

		props.resize(static_cast<decltype(props)::size_type>(supported_extension_count));
		vkEnumerateDeviceExtensionProperties(this->dev, nullptr, &supported_extension_count, props.data());
		extensions.resize(static_cast<decltype(props)::size_type>(supported_extension_count));

		std::transform(props.begin(), props.end(), extensions.begin(), [](const VkExtensionProperties& prop)->util::VkExtension{return prop.extensionName;});
		DeviceExtensionList exts;
		std::for_each(extensions.begin(), extensions.end(), [&exts](util::VkExtension ext)
		{
			DeviceExtension tz_ext = util::to_device_extension(ext);
			if(tz_ext != DeviceExtension::Count)
			{
				exts |= tz_ext;
			}
		});

		return exts;
	}
	
	PhysicalDeviceVendor PhysicalDevice::get_vendor() const
	{
		DeviceProps props = this->get_internal_device_props();
		return detail::to_tz_vendor(props.driver_props.driverID);
	}

	tz::BasicList<ImageFormat> PhysicalDevice::get_supported_surface_formats(const WindowSurface& surface) const
	{
		tz::BasicList<ImageFormat> fmts;

		std::vector<VkSurfaceFormatKHR> surf_fmts;
		std::uint32_t num_supported_formats;
		vkGetPhysicalDeviceSurfaceFormatsKHR(this->dev, surface.native(), &num_supported_formats, nullptr);
		surf_fmts.resize(static_cast<decltype(surf_fmts)::size_type>(num_supported_formats));
		vkGetPhysicalDeviceSurfaceFormatsKHR(this->dev, surface.native(), &num_supported_formats, surf_fmts.data());
		
		for(VkSurfaceFormatKHR surf_fmt : surf_fmts)
		{
			fmts.add(static_cast<ImageFormat>(surf_fmt.format));
		}
		return fmts;
	}

	bool PhysicalDevice::supports_image_colour_format(ImageFormat colour_format) const
	{
		return this->supports_image_format(colour_format, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT);
	}

	bool PhysicalDevice::supports_image_sampled_format(ImageFormat sampled_format) const
	{
		return this->supports_image_format(sampled_format, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
	}

	bool PhysicalDevice::supports_image_depth_format(ImageFormat depth_format) const
	{
		return this->supports_image_format(depth_format, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}
	VkPhysicalDevice PhysicalDevice::native() const
	{
		return this->dev;
	}

	PhysicalDevice::DeviceProps PhysicalDevice::get_internal_device_props() const
	{
		DeviceProps props;
		props.props.pNext = &props.driver_props;
		props.driver_props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;
		props.props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;

		vkGetPhysicalDeviceProperties2(this->dev, &props.props);
		return props;
	}

	bool PhysicalDevice::supports_image_format(ImageFormat format,VkFormatFeatureFlagBits feature_type) const
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(this->dev, static_cast<VkFormat>(format), &props);
		// Assume we always use optimal tiling.
		return (props.optimalTilingFeatures & feature_type) == feature_type;
	}

	PhysicalDeviceList get_all_devices(const VulkanInstance& instance)
	{
		PhysicalDeviceList devices;
		std::vector<VkPhysicalDevice> device_natives;

		std::uint32_t physical_device_count;
		vkEnumeratePhysicalDevices(instance.native(), &physical_device_count, nullptr);

		device_natives.resize(static_cast<std::size_t>(physical_device_count));
		vkEnumeratePhysicalDevices(instance.native(), &physical_device_count, device_natives.data());
		for(VkPhysicalDevice device_native : device_natives)
		{
			devices.emplace(device_native);
		}
		return devices;
	}

	PhysicalDeviceList get_all_devices()
	{
		return get_all_devices(vk2::get());
	}
}

#endif // TZ_VULKAN
