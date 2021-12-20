#include "gl/impl/backend/vk/tz_vulkan.hpp"
#include "gl/impl/backend/vk2/features.hpp"
#include "gl/impl/backend/vk2/image_format.hpp"
#include "gl/impl/frontend/common/shader.hpp"
#if TZ_VULKAN
#include "gl/impl/backend/vk2/hardware/physical_device.hpp"
#include <vector>

namespace tz::gl::vk2
{
	PhysicalDeviceSurfaceCapabilityInfo::PhysicalDeviceSurfaceCapabilityInfo(VkSurfaceCapabilitiesKHR vk_capabilities):
	min_image_count(vk_capabilities.minImageCount),
	max_image_count(vk_capabilities.maxImageCount),
	current_transform(vk_capabilities.currentTransform),
	maybe_surface_dimensions(std::nullopt)
	{
		// currentExtent is the current width and height of the surface, or the special value (0xFFFFFFFF, 0xFFFFFFFF) indicating that the surface size will be determined by the extent of a swapchain targeting the surface.
		const VkExtent2D cur = vk_capabilities.currentExtent;
		constexpr std::uint32_t special_value = 0xFFFFFFFF;
		if(cur.width == special_value && cur.height == special_value)
		{
			// Leave maybe_surface_dimensions as nullopt as it's upto the swapchain now.
		}
		else
		{
			// Retrieve the SwapchainExtent corresponding to the width and height of the surface.
			SwapchainExtent dims;
			dims.current_extent = tz::Vector<std::uint32_t, 2>{cur.width, cur.height};
			const VkExtent2D min = vk_capabilities.minImageExtent;
			const VkExtent2D max = vk_capabilities.maxImageExtent;
			dims.min_image_extent = tz::Vector<std::uint32_t, 2>{min.width, min.height};
			dims.max_image_extent = tz::Vector<std::uint32_t, 2>{max.width, max.height};
			this->maybe_surface_dimensions = dims;
		}
	}

	namespace detail
	{
		DeviceFeatureField to_feature_field(DeviceFeatureInfo features)
		{
			DeviceFeatureField ret;

			if(features.features.features.multiDrawIndirect)
			{
				ret |= DeviceFeature::MultiDrawIndirect;
			}
			if(features.descriptor_indexing_features.descriptorBindingStorageImageUpdateAfterBind
			&& features.descriptor_indexing_features.descriptorBindingStorageBufferUpdateAfterBind
			&& features.descriptor_indexing_features.descriptorBindingSampledImageUpdateAfterBind
			&& features.descriptor_indexing_features.descriptorBindingUpdateUnusedWhilePending
			&& features.descriptor_indexing_features.descriptorBindingPartiallyBound
			&& features.descriptor_indexing_features.descriptorBindingVariableDescriptorCount)
			{
				ret |= DeviceFeature::BindlessDescriptors;
			}
			if(features.features12.timelineSemaphore)
			{
				ret |= DeviceFeature::TimelineSemaphores;
			}
		
			return ret;
		}

		DeviceFeatureInfo from_feature_field(const DeviceFeatureField& feature_field)
		{
			DeviceFeatureInfo info;
			info.features.features.multiDrawIndirect = feature_field.contains(DeviceFeature::MultiDrawIndirect) ? VK_TRUE : VK_FALSE;
			info.features12.timelineSemaphore = feature_field.contains(DeviceFeature::TimelineSemaphores) ? VK_TRUE : VK_FALSE;

			if(feature_field.contains(DeviceFeature::BindlessDescriptors))
			{
				info.descriptor_indexing_features.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
				info.descriptor_indexing_features.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
				info.descriptor_indexing_features.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
				info.descriptor_indexing_features.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
				info.descriptor_indexing_features.descriptorBindingPartiallyBound = VK_TRUE;
				info.descriptor_indexing_features.descriptorBindingVariableDescriptorCount = VK_TRUE;
			}
			return info;
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

	PhysicalDevice::PhysicalDevice(VkPhysicalDevice native, const VulkanInstance& instance):
	dev(native),
	instance(&instance){}

	DeviceFeatureField PhysicalDevice::get_supported_features() const
	{
		tz_assert(!this->is_null(), "This was PhysicalDevice::null()");
		detail::DeviceFeatureInfo feature_info;
		vkGetPhysicalDeviceFeatures2(this->dev, &feature_info.features);

		return detail::to_feature_field(feature_info);
	}

	DeviceExtensionList PhysicalDevice::get_supported_extensions() const
	{
		tz_assert(!this->is_null(), "This was PhysicalDevice::null()");
		std::uint32_t supported_extension_count;
		vkEnumerateDeviceExtensionProperties(this->dev, nullptr, &supported_extension_count, nullptr);
		std::vector<VkExtensionProperties> props;
		std::vector<util::VkExtension> extensions; 

		props.resize(static_cast<decltype(props)::size_type>(supported_extension_count));
		vkEnumerateDeviceExtensionProperties(this->dev, nullptr, &supported_extension_count, props.data());
		extensions.resize(supported_extension_count);
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
		tz_assert(!this->is_null(), "This was PhysicalDevice::null()");
		DeviceProps props = this->get_internal_device_props();
		return detail::to_tz_vendor(props.driver_props.driverID);
	}

	tz::BasicList<ImageFormat> PhysicalDevice::get_supported_surface_formats() const
	{
		tz_assert(!this->is_null(), "This was PhysicalDevice::null()");
		tz_assert(this->instance != nullptr, "PhysicalDevice is not aware of its vulkan instance");
		tz_assert(this->instance->has_surface(), "PhysicalDevice belongs to a VulkanInstance that does not have a WindowSurface attached. Please submit a bug report");
		const WindowSurface& surface = this->instance->get_surface();
		// We can assume that this->instance == surface.get->instance() meaning that the VulkanInstance supports the "VK_KHR_surface" util::VkExtension which is what we need for vkGetPhysicalDeviceSurfaceFormatsKHR. For this reason we don't check for the extensions availability and assume everything is ok. 
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

	tz::BasicList<SurfacePresentMode> PhysicalDevice::get_supported_surface_present_modes() const
	{
		tz_assert(!this->is_null(), "This was PhysicalDevice::null()");
		tz_assert(this->instance != nullptr, "PhysicalDevice is not aware of its vulkan instance");
		tz_assert(this->instance->has_surface(), "PhysicalDevice belongs to a VulkanInstance that does not have a WindowSurface attached. Please submit a bug report");
		const WindowSurface& surface = this->instance->get_surface();
		// We can assume that this->instance == surface.get->instance() meaning that the VulkanInstance supports the "VK_KHR_surface" util::VkExtension which is what we need for vkGetPhysicalDeviceSurfaceFormatsKHR. For this reason we don't check for the extensions availability and assume everything is ok. 
		tz::BasicList<SurfacePresentMode> presents;
		tz::BasicList<VkPresentModeKHR> present_natives;

		std::uint32_t present_mode_count = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(this->dev, surface.native(), &present_mode_count, nullptr);
		present_natives.resize(present_mode_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(this->dev, surface.native(), &present_mode_count, present_natives.data());

		for(VkPresentModeKHR present_native : present_natives)
		{
			presents.add(static_cast<SurfacePresentMode>(present_native));
		}
		return presents;
	}

	PhysicalDeviceSurfaceCapabilityInfo PhysicalDevice::get_surface_capabilities() const
	{
		tz_assert(!this->is_null(), "This was PhysicalDevice::null()");
		tz_assert(this->instance != nullptr, "PhysicalDevice is not aware of its vulkan instance");
		tz_assert(this->instance->has_surface(), "PhysicalDevice belongs to a VulkanInstance that does not have a WindowSurface attached. Please submit a bug report");
		const WindowSurface& surface = this->instance->get_surface();
		// We can assume that this->instance == surface.get->instance() meaning that the VulkanInstance supports the "VK_KHR_surface" util::VkExtension which is what we need for vkGetPhysicalDeviceSurfaceFormatsKHR. For this reason we don't check for the extensions availability and assume everything is ok. 
		VkSurfaceCapabilitiesKHR vk_caps;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(this->dev, surface.native(), &vk_caps);
		return {vk_caps};
	}

	bool PhysicalDevice::supports_image_colour_format(ImageFormat colour_format) const
	{
		tz_assert(!this->is_null(), "This was PhysicalDevice::null()");
		return this->supports_image_format(colour_format, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT);
	}

	bool PhysicalDevice::supports_image_sampled_format(ImageFormat sampled_format) const
	{
		tz_assert(!this->is_null(), "This was PhysicalDevice::null()");
		return this->supports_image_format(sampled_format, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
	}

	bool PhysicalDevice::supports_image_depth_format(ImageFormat depth_format) const
	{
		tz_assert(!this->is_null(), "This was PhysicalDevice::null()");
		return this->supports_image_format(depth_format, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}

	const VulkanInstance& PhysicalDevice::get_instance() const
	{
		tz_assert(this->instance != nullptr, "PhysicalDevice had nullptr VulkanInstance. Please submit a bug report");
		return *this->instance;
	}

	PhysicalDevice::NativeType PhysicalDevice::native() const
	{
		return this->dev;
	}

	PhysicalDevice PhysicalDevice::null()
	{
		return {};
	}

	bool PhysicalDevice::is_null() const
	{
		return this->dev == VK_NULL_HANDLE;
	}

	PhysicalDevice::DeviceProps PhysicalDevice::get_internal_device_props() const
	{
		tz_assert(!this->is_null(), "This was PhysicalDevice::null()");
		DeviceProps props;
		props.props.pNext = &props.driver_props;
		props.driver_props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;
		props.props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;

		vkGetPhysicalDeviceProperties2(this->dev, &props.props);
		return props;
	}

	PhysicalDevice::PhysicalDevice():
	dev(VK_NULL_HANDLE),
	instance(nullptr)
	{

	}

	bool PhysicalDevice::supports_image_format(ImageFormat format,VkFormatFeatureFlagBits feature_type) const
	{
		tz_assert(!this->is_null(), "This was PhysicalDevice::null()");
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
			devices.emplace(device_native, instance);
		}
		return devices;
	}

	PhysicalDeviceList get_all_devices()
	{
		return get_all_devices(vk2::get());
	}
}

#endif // TZ_VULKAN
