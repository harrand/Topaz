#include "gl/impl/backend/vk2/hardware/physical_device.hpp"
#if TZ_VULKAN
#include "gl/impl/backend/vk2/logical_device.hpp"

namespace tz::gl::vk2
{
	LogicalDevice::LogicalDevice(PhysicalDevice physical_device, ExtensionList enabled_extensions, PhysicalDeviceFeatureField enabled_features):
	dev(VK_NULL_HANDLE),
	physical_device(physical_device),
	enabled_extensions(enabled_extensions),
	enabled_features(enabled_features)
	{
		// Firstly, let's retrieve some information about the PhysicalDevice's queue families. Note that its API doesn't expose this to the end-user, so we have to do this ourselves.
		std::vector<QueueFamilyInfo> queue_families;
		{
			std::vector<VkQueueFamilyProperties> queue_family_props;

			std::uint32_t queue_family_property_count;
			vkGetPhysicalDeviceQueueFamilyProperties(this->physical_device.native(), &queue_family_property_count, nullptr);
			using SizeType = decltype(queue_family_props)::size_type;
			queue_family_props.resize(static_cast<SizeType>(queue_family_property_count));

			vkGetPhysicalDeviceQueueFamilyProperties(this->physical_device.native(), &queue_family_property_count, queue_family_props.data());
			// Queue Family Index == i, where queue_family_props[i] makes sense.
			std::for_each(queue_family_props.begin(), queue_family_props.end(), [&queue_families](VkQueueFamilyProperties prop)
			{
				QueueFamilyInfo info;
				info.family_size = prop.queueCount;
				info.types = {};
				if(prop.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					info.types |= QueueFamilyType::Graphics;
				}
				if(prop.queueFlags & VK_QUEUE_COMPUTE_BIT)
				{
					info.types |= QueueFamilyType::Compute;
				}
				if(prop.queueFlags & VK_QUEUE_TRANSFER_BIT)
				{
					info.types |= QueueFamilyType::Transfer;
				}
				queue_families.push_back(info);
			});
		}
		// Now create the VkDeviceQueueCreateInfos
		std::vector<VkDeviceQueueCreateInfo> queue_creates;
		for(std::uint32_t qf_index = 0; qf_index < queue_families.size(); qf_index++)
		{
			VkDeviceQueueCreateInfo& queue_create = queue_creates.emplace_back();
			queue_create.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create.queueFamilyIndex = qf_index;
			queue_create.queueCount = queue_families[static_cast<decltype(queue_families)::size_type>(qf_index)].family_size;
		}
		// This is when we actually can create the LogicalDevice.
		VkDeviceCreateInfo create{};
		create.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		// Both of these are deprecated and ignored. Validation laters are exclusively instance-level, not device-level.
		create.enabledLayerCount = 0;
		create.ppEnabledLayerNames = nullptr;

		// Use the queue family data we sorted out earlier.
		create.queueCreateInfoCount = queue_creates.size();
		create.pQueueCreateInfos = queue_creates.data();

		// Now deal with extensions.
		VkExtensionList vk_extensions;
		#if TZ_DEBUG
			ExtensionList debug_supported_extensions = this->physical_device.get_supported_extensions();		
		#endif
		for(Extension ext : this->enabled_extensions)
		{
			tz_assert(ext != Extension::Count, "Invalid extension passed into LogicalDevice ctor Extension::Count");
			tz_assert(debug_supported_extensions.contains(ext), "LogicalDevice attempted to use the extension %s, but the correspnding PhysicalDevice does not support it. Submit a bug report.", util::to_vk_extension(ext));
			vk_extensions.add(util::to_vk_extension(ext));
		}
		create.enabledExtensionCount = this->enabled_extensions.count();
		create.ppEnabledExtensionNames = vk_extensions.data();	

		// And finally, features.
		#if TZ_DEBUG
			PhysicalDeviceFeatureField debug_supported_features = this->physical_device.get_supported_features();
			std::for_each(this->enabled_features.begin(), this->enabled_features.end(), [&debug_supported_features](const PhysicalDeviceFeature& feature)
			{
				tz_assert(debug_supported_features.contains(feature), "LogicalDevice attempted to use a feature, but the corresponding PhysicalDevice does not support it. Submit a bug report.");
			});		
		#endif
		VkPhysicalDeviceFeatures features = detail::from_feature_field(this->enabled_features);
		create.pEnabledFeatures = &features;
		VkResult res = vkCreateDevice(this->physical_device.native(), &create, nullptr, &this->dev);
		switch(res)
		{
			case VK_SUCCESS: break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				tz_error("Ran out of host memory whilst trying to create LogicalDevice");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				tz_error("Ran out of device memory whilst trying to create LogicalDevice");
			break;
			case VK_ERROR_INITIALIZATION_FAILED:
			[[fallthrough]];
			default:
				tz_error("Failed to create LogicalDevice, but for unknown reason. Ensure that your machine meets the system requirements.");
			break;
			case VK_ERROR_EXTENSION_NOT_PRESENT:
				tz_error("Failed to create LogicalDevice because an extension was enabled but not available. An assert should've popped earlier - If this is the first error you've seen, submit a bug report.");
			break;
			case VK_ERROR_FEATURE_NOT_PRESENT:
				tz_error("Failed to create LogicalDevice because a feature was enabled but not supported by the PhysicalDevice. An assert should've popped earlier - If this is the first error you've seen, submit a bug report");
			break;
			case VK_ERROR_TOO_MANY_OBJECTS:
				tz_error("Failed to create LogicalDevice because too many of such objects");
			break;
			case VK_ERROR_DEVICE_LOST:
				tz_error("Device lost whilst trying to create a LogicalDevice. Possible hardware fault. Please be aware: Device loss is extremely serious and further attempts to run the engine may cause serious hazards, such as operating system crash. Submit a bug report but do not attempt to reproduce the issue.");
			break;
		}
	}

	const PhysicalDevice& LogicalDevice::get_hardware() const
	{
		return this->physical_device;
	}

	const ExtensionList& LogicalDevice::get_extensions() const
	{
		return this->enabled_extensions;
	}


	VkDevice LogicalDevice::native() const
	{
		return this->dev;
	}
}

#endif // TZ_VULKAN
