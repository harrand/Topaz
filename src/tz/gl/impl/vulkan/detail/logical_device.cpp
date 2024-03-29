#if TZ_VULKAN
#include "tz/core/profile.hpp"
#include "tz/gl/impl/vulkan/detail/features.hpp"
#include "tz/gl/impl/vulkan/detail/hardware/physical_device.hpp"
#include "tz/gl/impl/vulkan/detail/logical_device.hpp"

namespace tz::gl::vk2
{
	void QueueStorage::init(std::span<const QueueFamilyInfo> queue_families, const LogicalDevice& device)
	{
		TZ_PROFZONE("Vulkan Backend - QueueStorage Initialisation", 0xFFAA0000);
		for(std::uint32_t i = 0; std::cmp_less(i, queue_families.size()); i++)
		{
			List& hardware_queue_family = this->hardware_queue_families.emplace();
			for(std::uint32_t j = 0; std::cmp_less(j, queue_families[i].family_size); j++)
			{
				hardware::QueueInfo info;
				info.dev = &device;
				info.queue_family_idx = i;
				info.queue_idx = j;

				hardware_queue_family.emplace(info, queue_families[i]);
			}
		}
	}

	void QueueStorage::notify_device_moved(const LogicalDevice& new_device)
	{
		for(QueueStorage::List& queue_list : this->hardware_queue_families)
		{
			for(QueueData& queue_data : queue_list)
			{
				queue_data.queue.set_logical_device(new_device);
			}
		}
	}

	const hardware::Queue* QueueStorage::request_queue(QueueRequest request) const
	{
		TZ_PROFZONE("Vulkan Backend - Queue Storage Request", 0xFFAA0000);
		auto queue_satisfies_request = [request](const QueueFamilyInfo& info)
		{
			// If we request present support, the queue family *must* support it.
			if(request.present_support && !info.present_support)
			{
				return false;
			}
			// Otherwise ensure the type field matches.
			return std::all_of(request.field.begin(), request.field.end(), [info](QueueFamilyType type)
			{
				return info.types.contains(type);
			});
		};
		for(const QueueStorage::List& queue_family : this->hardware_queue_families)
		{
			if(queue_family.empty())
			{
				continue;
			}
			const QueueData& qdata = queue_family.front();
			if(queue_satisfies_request(qdata.family))
			{
				return &qdata.queue;	
			}
		}
		return nullptr;	
	}
	
	hardware::Queue* QueueStorage::request_queue(QueueRequest request)
	{
		auto queue_satisfies_request = [request](const QueueFamilyInfo& info)
		{
			// If we request present support, the queue family *must* support it.
			if(request.present_support && !info.present_support)
			{
				return false;
			}
			// Otherwise ensure the type field matches.
			return std::all_of(request.field.begin(), request.field.end(), [info](QueueFamilyType type)
			{
				return info.types.contains(type);
			});
		};
		for(QueueStorage::List& queue_family : this->hardware_queue_families)
		{
			if(queue_family.empty())
			{
				continue;
			}
			QueueData& qdata = queue_family.front();
			if(queue_satisfies_request(qdata.family))
			{
				return &qdata.queue;	
			}
		}
		return nullptr;	
	}

	QueueStorage::QueueData::QueueData(hardware::QueueInfo queue_info, QueueFamilyInfo family_info):
	queue(queue_info),
	family(family_info){}

	QueueStorage::QueueData::QueueData(QueueData&& move):
	queue(std::move(move.queue)),
	family()
	{
		std::swap(this->family, move.family);
	}

	QueueStorage::QueueData& QueueStorage::QueueData::operator=(QueueData&& rhs)
	{
		std::swap(this->queue, rhs.queue);
		std::swap(this->family, rhs.family);
		return *this;
	}

	bool QueueStorage::QueueData::operator==(const QueueStorage::QueueData& rhs)
	{
		return this->queue == rhs.queue;
	}

	LogicalDevice::LogicalDevice(LogicalDeviceInfo device_info):
	dev(VK_NULL_HANDLE),
	physical_device(device_info.physical_device),
	enabled_extensions(device_info.extensions),
	enabled_features(device_info.features),
	queue_families(),
	vma_allocator()
	{
		TZ_PROFZONE("Vulkan Backend - LogicalDevice Create", 0xFFAA0000);
		tz::assert(this->physical_device.native() != VK_NULL_HANDLE, "Null PhysicalDevice provided to LogicalDevice ctor. Submit a bug report.");
		// Firstly, let's retrieve some information about the PhysicalDevice's queue families. Note that its API doesn't expose this to the end-user, so we have to do this ourselves.
		{
			std::vector<VkQueueFamilyProperties> queue_family_props;

			std::uint32_t queue_family_property_count;
			vkGetPhysicalDeviceQueueFamilyProperties(this->physical_device.native(), &queue_family_property_count, nullptr);
			queue_family_props.resize(queue_family_property_count);

			vkGetPhysicalDeviceQueueFamilyProperties(this->physical_device.native(), &queue_family_property_count, queue_family_props.data());
			// Queue Family Index == i, where queue_family_props[i] makes sense.
			for(std::uint32_t queue_family_index = 0; std::cmp_less(queue_family_index, queue_family_props.size()); queue_family_index++)
			{

				VkQueueFamilyProperties prop = queue_family_props[queue_family_index];
				QueueFamilyInfo info;
				info.family_size = prop.queueCount;
				info.types = {};
				info.present_support = false;
				if(prop.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					info.types |= QueueFamilyType::graphics;
				}
				if(prop.queueFlags & VK_QUEUE_COMPUTE_BIT)
				{
					info.types |= QueueFamilyType::compute;
				}
				if(prop.queueFlags & VK_QUEUE_TRANSFER_BIT)
				{
					info.types |= QueueFamilyType::transfer;
				}

				const VulkanInstance& vk_inst = this->get_hardware().get_instance();
				tz::assert(vk_inst.has_surface(), "VulkanInstance did not have a surface associated with it.");
				const WindowSurface& surf = vk_inst.get_surface();
				VkBool32 present_support;
				vkGetPhysicalDeviceSurfaceSupportKHR(this->physical_device.native(), queue_family_index, surf.native(), &present_support);
				info.present_support = present_support == VK_TRUE;

				this->queue_families.push_back(info);
			};
		}
		// Now create the VkDeviceQueueCreateInfos
		std::vector<VkDeviceQueueCreateInfo> queue_creates;
		std::vector<std::vector<float>> queue_priorities;
		queue_priorities.resize(this->queue_families.size());
		for(std::uint32_t qf_index = 0; qf_index < this->queue_families.size(); qf_index++)
		{
			VkDeviceQueueCreateInfo& queue_create = queue_creates.emplace_back();
			queue_create.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create.queueFamilyIndex = qf_index;
			queue_create.queueCount = this->queue_families[qf_index].family_size;
			queue_priorities[qf_index].resize(queue_create.queueCount, 1.0f);
			queue_create.pQueuePriorities = queue_priorities[qf_index].data();
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
		util::VkExtensionList vk_extensions;
		#if TZ_DEBUG
			DeviceExtensionList debug_supported_extensions = this->physical_device.get_supported_extensions();		
		#endif
		for(DeviceExtension ext : this->enabled_extensions)
		{
			tz::assert(ext != DeviceExtension::Count, "Invalid extension passed into LogicalDevice ctor Extension::Count");
			#if TZ_DEBUG
				tz::assert(debug_supported_extensions.contains(ext), "LogicalDevice attempted to use the extension %s, but the correspnding PhysicalDevice does not support it. Submit a bug report.", util::to_vk_extension(ext));
			#endif
			vk_extensions.add(util::to_vk_extension(ext));
		}
		create.enabledExtensionCount = this->enabled_extensions.count();
		create.ppEnabledExtensionNames = vk_extensions.data();	

		// And finally, features.
		#if TZ_DEBUG
			DeviceFeatureField debug_supported_features = this->physical_device.get_supported_features();
			std::for_each(this->enabled_features.begin(), this->enabled_features.end(), [&debug_supported_features](const DeviceFeature& feature)
			{
				tz::assert(debug_supported_features.contains(feature), "LogicalDevice attempted to use a feature, but the corresponding PhysicalDevice does not support it. Submit a bug report.");
			});		
		#endif
		detail::DeviceFeatureInfo features = detail::from_feature_field(this->enabled_features);
		create.pEnabledFeatures = nullptr;
		create.pNext = &features.features;
		VkResult res = vkCreateDevice(this->physical_device.native(), &create, nullptr, &this->dev);
		switch(res)
		{
			case VK_SUCCESS: break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				tz::error("Ran out of host memory whilst trying to create LogicalDevice");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				tz::error("Ran out of device memory whilst trying to create LogicalDevice");
			break;
			case VK_ERROR_INITIALIZATION_FAILED:
			[[fallthrough]];
			default:
				tz::error("Failed to create LogicalDevice, but for unknown reason. Ensure that your machine meets the system requirements.");
			break;
			case VK_ERROR_EXTENSION_NOT_PRESENT:
				tz::error("Failed to create LogicalDevice because an extension was enabled but not available. An assert should've popped earlier - If this is the first error you've seen, submit a bug report.");
			break;
			case VK_ERROR_FEATURE_NOT_PRESENT:
				tz::error("Failed to create LogicalDevice because a feature was enabled but not supported by the PhysicalDevice. An assert should've popped earlier - If this is the first error you've seen, submit a bug report");
			break;
			case VK_ERROR_TOO_MANY_OBJECTS:
				tz::error("Failed to create LogicalDevice because too many of such objects. Pleas submit a bug report.");
			break;
			case VK_ERROR_DEVICE_LOST:
				tz::error("device lost whilst trying to create a LogicalDevice. Possible hardware fault. Please be aware: device loss is serious and further attempts to run the engine may cause serious hazards, such as operating system crash. Submit a bug report but do not attempt to reproduce the issue.");
			break;
		}
		// We'll retrieve all the VkQueues now so we don't have to deal with it later.
		this->queue_storage.init(this->queue_families, *this);

		// Now we'll create a VMA allocator.
		VmaAllocatorCreateInfo vma_create
		{
			.flags = 0,
			.physicalDevice = this->get_hardware().native(),
			.device = this->native(),
			.preferredLargeHeapBlockSize = 0,
			.pAllocationCallbacks = nullptr,
			.pDeviceMemoryCallbacks = nullptr,
			.pHeapSizeLimit = nullptr,
			.pVulkanFunctions = nullptr,
			.instance = this->get_hardware().get_instance().native(),
			.vulkanApiVersion = util::tz_to_vk_version(vulkan_version),
			.pTypeExternalMemoryHandleTypes = nullptr
		};

		VkResult alloc_res = vmaCreateAllocator(&vma_create, &this->vma_allocator);
		switch(alloc_res)
		{
			case VK_SUCCESS:

			break;
			default:
				tz::error("Failed to create VMA allocator for unknown reason. Please submit a bug report.");
			break;
		}
	}

	LogicalDevice::LogicalDevice(LogicalDevice&& move):
	dev(VK_NULL_HANDLE),
	physical_device(PhysicalDevice::null()),
	enabled_extensions(),
	enabled_features(),
	queue_families(),
	queue_storage(),
	vma_allocator()
	{
		*this = std::move(move);
	}

	LogicalDevice::~LogicalDevice()
	{
		if(this->dev != VK_NULL_HANDLE)
		{
			vmaDestroyAllocator(this->vma_allocator);
			vkDestroyDevice(this->dev, nullptr);
			this->dev = VK_NULL_HANDLE;
		}
	}

	LogicalDevice& LogicalDevice::operator=(LogicalDevice&& rhs)
	{
		std::swap(this->dev, rhs.dev);
		std::swap(this->physical_device, rhs.physical_device);
		std::swap(this->enabled_extensions, rhs.enabled_extensions);
		std::swap(this->enabled_features, rhs.enabled_features);
		std::swap(this->queue_families, rhs.queue_families);
		std::swap(this->queue_storage, rhs.queue_storage);
		std::swap(this->vma_allocator, rhs.vma_allocator);

		this->queue_storage.notify_device_moved(*this);
		rhs.queue_storage.notify_device_moved(rhs);
		return *this;
	}

	const PhysicalDevice& LogicalDevice::get_hardware() const
	{
		return this->physical_device;
	}

	const DeviceExtensionList& LogicalDevice::get_extensions() const
	{
		return this->enabled_extensions;
	}

	const DeviceFeatureField& LogicalDevice::get_features() const
	{
		return this->enabled_features;
	}

	void LogicalDevice::wait_until_idle() const
	{
		vkDeviceWaitIdle(this->dev);
	}

	const hardware::Queue* LogicalDevice::get_hardware_queue(QueueRequest request) const
	{
		return this->queue_storage.request_queue(request);
	}

	hardware::Queue* LogicalDevice::get_hardware_queue(QueueRequest request) 
	{
		return this->queue_storage.request_queue(request);
	}

	VkDevice LogicalDevice::native() const
	{
		return this->dev;
	}

	VmaAllocator LogicalDevice::vma_native() const
	{
		return this->vma_allocator;
	}

	LogicalDevice LogicalDevice::null()
	{
		return {};
	}

	bool LogicalDevice::is_null() const
	{
		return this->dev == VK_NULL_HANDLE;
	}

	bool LogicalDevice::operator==(const LogicalDevice& rhs) const
	{
		return this->dev == rhs.dev;
	}

	LogicalDevice::LogicalDevice():
	dev(VK_NULL_HANDLE),
	physical_device(PhysicalDevice::null()),
	enabled_extensions(),
	enabled_features(),
	queue_families(),
	queue_storage()
	{}
}

#endif // TZ_VULKAN
