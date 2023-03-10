#if TZ_VULKAN
#include "tz/gl/impl/vulkan/device2.hpp"
#include "tz/gl/impl/vulkan/detail/semaphore.hpp"
#include "tz/gl/impl/vulkan/detail/fence.hpp"
#include "tz/gl/impl/vulkan/detail/tz_vulkan.hpp"
#include "tz/core/profile.hpp"
#include <algorithm>

namespace tz::gl
{
//--------------------------------------------------------------------------------------------------
	device_window::device_window(const vk2::LogicalDevice& device):
	ldev(&device)
	{
		// choose initial settings and create swapchain + depth.
		TZ_PROFZONE("vk - device_window create", 0xFFAA0000);
		const vk2::PhysicalDevice& hardware = this->ldev->get_hardware();
		const vk2::VulkanInstance& vkinst = hardware.get_instance();
		tz::assert(vkinst.has_surface(), "Tried to create device_window, but vulkan instance has no surface.");

		// Choose a present mode we support.
		vk2::SurfacePresentMode present_modes[] = {vk2::SurfacePresentMode::Mailbox};
		auto iter = std::find_if(std::begin(present_modes), std::end(present_modes),
		[&hardware](vk2::SurfacePresentMode mode)
		{
			return hardware.get_supported_surface_present_modes().contains(mode);
		});
		tz::assert(iter != std::end(present_modes), "The hardware does not support any of the surface present modes we support. Your hardware is not supported.");
		vk2::SurfacePresentMode chosen_present_mode = *iter;

		// Choose the number of swapchain images.
		constexpr std::uint32_t preferred_swapchain_images = 3u;
		const auto& surface_capabilities = hardware.get_surface_capabilities();
		std::uint32_t swapchain_image_count = std::clamp(preferred_swapchain_images, surface_capabilities.min_image_count, surface_capabilities.max_image_count);

		// TODO: More choice over format. Maybe prefer BGRA32?
		vk2::image_format chosen_format = hardware.get_supported_surface_formats().front();
		
		this->swapchain = vk2::Swapchain
		{{
			.device = this->ldev,
			.swapchain_image_count_minimum = swapchain_image_count,
			.format = chosen_format,
			.present_mode = chosen_present_mode
		}};
		this->dimensions_cache = tz::window().get_dimensions();
		this->make_depth_image();
		this->debug_annotate_resources();
	}

	const vk2::Swapchain& device_window::get_swapchain() const
	{
		return this->swapchain;
	}

	vk2::Swapchain& device_window::get_swapchain()
	{
		return this->swapchain;
	}

	const vk2::Image& device_window::get_depth_image() const
	{
		return this->device_depth;
	}

	vk2::Image& device_window::get_depth_image()
	{
		return this->device_depth;
	}

	std::size_t device_window::acquire_image(const vk2::Swapchain::ImageAcquisition& acquire)
	{
		if(!this->recent_acquire.has_value())
		{
			this->recent_acquire = this->swapchain.acquire_image(acquire);
		}
		return this->recent_acquire.value().image_index;
	}

	vk2::hardware::Queue::PresentResult device_window::present_image(vk2::hardware::Queue& present_queue, std::span<const vk2::BinarySemaphore> wait_semaphores)
	{
		tz::assert(this->recent_acquire.has_value(), "Attempting to present image, but no image has been previously acquired. Logic error.");
		tz::basic_list<const vk2::BinarySemaphore*> waits;
		waits.resize(wait_semaphores.size());
		std::transform(wait_semaphores.begin(), wait_semaphores.end(), waits.begin(), [](const vk2::BinarySemaphore& sem){return &sem;});
		return present_queue.present
		({
			.wait_semaphores = waits,
	   		.swapchain = &this->swapchain,
			.swapchain_image_index = this->recent_acquire.value().image_index
		});
	}

	void device_window::make_depth_image()
	{
		tz::assert(!this->swapchain.is_null());
		this->device_depth =
		{{
			.device = this->ldev,
			.format = vk2::image_format::Depth32_SFloat,
			.dimensions = this->swapchain.get_dimensions(),
			.usage = {vk2::ImageUsage::DepthStencilAttachment},
			.residency = vk2::MemoryResidency::GPU
		}};
	}

	void device_window::debug_annotate_resources()
	{
		for(auto& img : this->swapchain.get_images())
		{
			img.debug_set_name("Device Swapchain Image");
		}
		this->device_depth.debug_set_name("Device Depth Image");
	}

//--------------------------------------------------------------------------------------------------

	device_render_sync::device_render_sync(const vk2::LogicalDevice& ldev, const tz::gl::timeline_t& timeline):
	timeline(timeline),
	tsem(ldev, 0)
	{

	}

//--------------------------------------------------------------------------------------------------

	device_descriptor_pool::device_descriptor_pool(const vk2::LogicalDevice& device):
	ldev(&device)
	{
		this->another_pool();
	}

	vk2::DescriptorPool::UpdateRequest device_descriptor_pool::vk_make_update_request(unsigned int fingerprint)
	{
		return this->get_pool(fingerprint).make_update_request();
	}

	vk2::DescriptorPool::AllocationResult device_descriptor_pool::vk_allocate_sets(const vk2::DescriptorPool::Allocation& alloc, unsigned int fingerprint)
	{
		return this->impl_allocate_sets(alloc, fingerprint, 0);
	}

	void device_descriptor_pool::vk_update_sets(vk2::DescriptorPool::UpdateRequest update, unsigned int fingerprint)
	{
		this->get_pool(fingerprint).update_sets(update);
	}

	vk2::DescriptorPool& device_descriptor_pool::get_pool(unsigned int fingerprint)
	{
		tz::assert(this->fingerprint_to_pool_id.find(fingerprint) != this->fingerprint_to_pool_id.end(), "A renderer requested to write descriptor sets with fingerprint %u, but no such fingerprint was ever used to allocate sets.", fingerprint);
		return this->pools[this->fingerprint_to_pool_id.at(fingerprint)];
	}

	vk2::DescriptorPool::AllocationResult device_descriptor_pool::impl_allocate_sets(const vk2::DescriptorPool::Allocation& alloc, unsigned int fingerprint, unsigned int attempt)
	{
		// allocate using the most recently created descriptor pool
		// if allocation fails due to lack of memory, create a new descriptor pool and try again.
		// if it fails too many times in a row, we're gonna error out.
		constexpr unsigned int failure_attempt = 8;
		tz::assert(attempt < failure_attempt, "Failed to allocate descriptor sets for a renderer after %u attempts. Please submit a bug report", failure_attempt);

		vk2::DescriptorPool::AllocationResult ret = this->pools.back().allocate_sets(alloc);
		using ART = vk2::DescriptorPool::AllocationResult::AllocationResultType;
		if(ret.type == ART::AllocationSuccess)
		{
			this->fingerprint_to_pool_id[fingerprint] = this->pools.size();
			return ret;
		}
		else
		{
			if(ret.type == ART::FragmentedPool || ret.type == ART::PoolOutOfMemory)
			{
				// todooo: try to tailor the new pool to the sizes expected by the alloc request. if the alloc request is larger than our defaults, we will simply never succeed.
				this->another_pool();
				return this->impl_allocate_sets(alloc, fingerprint, attempt + 1);
			}
			if(ret.type == ART::FatalError)
			{
				tz::error("Fatal error occurred while trying to allocate descriptor sets for a renderer. Was on attempt %u of %u", attempt + 1, failure_attempt + 1);
			}
			else
			{
				tz::error("Unknown error occurred while trying to allocate descriptor sets for a renderer. Possible memory corruption? Was on attempt %u of %u", attempt + 1, failure_attempt + 1);
			}
			return {};
		}
	}

	void device_descriptor_pool::another_pool()
	{
		// note: this will not use 0 for the limits, but the defaults.
		this->another_pool(0, 0, 0);
	}

	void device_descriptor_pool::another_pool(std::size_t set_count, std::size_t buf_count, std::size_t img_count)
	{
		constexpr std::size_t min_pool_storage_buffer_count = 128;
		constexpr std::size_t min_pool_samplerimage_count = 128;
		constexpr std::size_t min_pool_set_count = 128;

		std::size_t pool_buf_count = std::max(min_pool_storage_buffer_count, buf_count);
		std::size_t pool_img_count = std::max(min_pool_samplerimage_count, img_count);
		std::size_t pool_set_count = std::max(min_pool_set_count, set_count);

		vk2::DescriptorPoolInfo::PoolLimits pool_lims;
		pool_lims.limits[vk2::DescriptorType::StorageBuffer] = pool_buf_count;
		pool_lims.limits[vk2::DescriptorType::ImageWithSampler] = pool_img_count;
		pool_lims.max_sets = pool_set_count;
		this->pools.emplace_back(vk2::DescriptorPoolInfo
		{
			.limits = pool_lims,
			.logical_device = this->ldev
		});
	}

//--------------------------------------------------------------------------------------------------

	device_command_pool::device_command_pool(vk2::LogicalDevice& device):
	ldev(&device)
	{
		this->graphics_queue = this->ldev->get_hardware_queue
		({
			.field = {vk2::QueueFamilyType::graphics},
	   		.present_support = false
   		});

		this->graphics_present_queue = this->ldev->get_hardware_queue
		({
			.field = {vk2::QueueFamilyType::graphics},
	   		.present_support = true
   		});

		this->compute_queue = this->ldev->get_hardware_queue
		({
			.field = {vk2::QueueFamilyType::compute},
	   		.present_support = false
   		});
	}

	vk2::CommandPool::AllocationResult device_command_pool::vk_allocate_commands(const vk2::CommandPool::Allocation& alloc, unsigned int fingerprint)
	{
		tz::assert(alloc.buffer_count > 0, "it's illegal to do a command buffer allocation of size zero. please submit a bug report.");
		return this->impl_allocate_commands(alloc, fingerprint, 0);
	}

	void device_command_pool::vk_free_commands(unsigned int fingerprint, std::size_t allocation_id, std::span<vk2::CommandBuffer> command_buffers)
	{
		tz::assert(this->fingerprint_allocation_history.find(fingerprint) != this->fingerprint_allocation_history.end(), "cannot free commands because there is no recorded history for fingerprint %u", fingerprint);
		auto& allocations = this->fingerprint_allocation_history.at(fingerprint);
		tz::assert(!allocations.empty(), "attempted to free command buffers of allocation-id %zu for fingerprint %u, but there were no allocations -- either a zero-allocation occurred (wouldve asserted earlier), or this has already been freed.");
		tz::assert(allocations.size() > allocation_id, "allocation id %zu for fingerprint %u is invalid - only %zu allocations are in the history for this fingerprint", allocation_id, fingerprint, allocations.size());
		tz::assert(allocations.size() == command_buffers.size(), "passed span for command buffers in free (size %zu) does not match the number of buffers that were allocated (%zu). the span does not match. logic error. please submit a bug report.");
		
		// do the free.
		vk2::CommandPool& pool = this->get_fitting_pool(this->fingerprint_alloc_types.at(fingerprint));
		tz::basic_list<vk2::CommandBuffer> buffers;
		for(std::size_t i = 0; i < command_buffers.size(); i++)
		{
			buffers.add(std::move(command_buffers[i]));
		}
		pool.free_buffers({.buffers = buffers, .type = vk2::CommandPool::AllocationResult::AllocationResultType::AllocationSuccess});
		
		// allocations have a buffer count of 0 when freed.
		allocations[allocation_id] = {.buffer_count = 0u};
	}

	void device_command_pool::vk_command_pool_touch(unsigned int fingerprint, fingerprint_info_t finfo)
	{
		this->fingerprint_alloc_types[fingerprint] = finfo;
	}

	void device_command_pool::vk_submit_and_run_commands_blocking(unsigned int fingerprint, std::size_t allocation_id, std::size_t buffer_id, const vk2::CommandBuffer& buffer)
	{
		vk2::Fence temp_fence
		{{
			.device = this->ldev
		}};

		#if TZ_DEBUG
			std::size_t debug_real_alloc_length = this->fingerprint_allocation_history[fingerprint].size();
			tz::assert(debug_real_alloc_length > buffer_id, "attempted to submit & run scratch command buffer id %zu at (fingerprint:allocid) %u:%zu. there are only %zu buffers in this allocation. please submit a bug report.", buffer_id, fingerprint, allocation_id, debug_real_alloc_length);
		#endif // TZ_DEBUG
	
		this->get_original_queue(this->fingerprint_alloc_types[fingerprint])->submit
		(vk2::hardware::Queue::SubmitInfo{
			.command_buffers = {&buffer},
	   		.waits = {},
	   		.signals = {},
	   		.execution_complete_fence = &temp_fence
		});	
		temp_fence.wait_until_signalled();
	}

	void device_command_pool::vk_submit_command(unsigned int fingerprint, std::size_t allocation_id, std::size_t buffer_id, vk2::hardware::Queue::SubmitInfo submit)
	{
		#if TZ_DEBUG
			std::size_t debug_real_alloc_length = this->fingerprint_allocation_history[fingerprint].size();
			tz::assert(debug_real_alloc_length > buffer_id, "attempted to submit & run scratch command buffer id %zu at (fingerprint:allocid) %u:%zu. there are only %zu buffers in this allocation. please submit a bug report.", buffer_id, fingerprint, allocation_id, debug_real_alloc_length);
		#endif // TZ_DEBUG
	
		this->get_original_queue(this->fingerprint_alloc_types[fingerprint])->submit(submit);
	}

	vk2::CommandPool::AllocationResult device_command_pool::impl_allocate_commands(const vk2::CommandPool::Allocation& alloc, unsigned int fingerprint, unsigned int attempt)
	{
		// allocate using the most recently created descriptor pool
		// if allocation fails due to lack of memory, create a new descriptor pool and try again.
		// if it fails too many times in a row, we're gonna error out.
		constexpr unsigned int failure_attempt = 8;
		tz::assert(attempt < failure_attempt, "Failed to allocate command buffers for a renderer after %u attempts. Please submit a bug report", failure_attempt);
		vk2::CommandPool& pool = this->get_fitting_pool(this->fingerprint_alloc_types.at(fingerprint));

		vk2::CommandPool::AllocationResult ret = pool.allocate_buffers(alloc);
		using ART = vk2::CommandPool::AllocationResult::AllocationResultType;
		if(ret.type == ART::AllocationSuccess)
		{
			this->fingerprint_allocation_history[fingerprint].push_back(alloc);
			return ret;
		}
		else
		{
			if(ret.type == ART::FatalError)
			{
				tz::error("Fatal error occurred while trying to allocate command buffers for a renderer. Was on attempt %u of %u", attempt + 1, failure_attempt + 1);
			}
			else
			{
				// todo: attempt to recycle command buffer allocations from a fingerprint that is now marked as dead. NYI.
				tz::error("Unknown error occurred while trying to allocate command buffers for a renderer. This could've been mitigated by command buffer recycling from dead renderers, which is NYI.");
				return {};
				//this->another_pool();
				//return this->impl_allocate_commands(alloc, fingerprint, attempt + 1);
			}
			return {};
		}
	}

	vk2::CommandPool& device_command_pool::get_fitting_pool(const fingerprint_info_t& finfo)
	{
		if(finfo.compute)
		{
			return this->compute_commands;
		}
		if(finfo.requires_present)
		{
			return this->graphics_present_commands;
		}
		return this->graphics_commands;
	}

	vk2::hardware::Queue* device_command_pool::get_original_queue(const fingerprint_info_t& finfo)
	{
		if(finfo.compute)
		{
			return this->compute_queue;
		}
		if(finfo.requires_present)
		{
			return this->graphics_present_queue;
		}
		return this->graphics_queue;
	}

//--------------------------------------------------------------------------------------------------

	unsigned int rate_device(const vk2::PhysicalDevice device)
	{
		unsigned int rating = 0u;
		const vk2::PhysicalDeviceInfo info = device.get_info();
		switch(info.type)
		{
			case vk2::PhysicalDeviceType::IntegratedGPU:
				rating += 500;
			break;
			case vk2::PhysicalDeviceType::DiscreteGPU:
				rating += 2000;
			break;
			case vk2::PhysicalDeviceType::VirtualGPU:
				rating += 300;
			break;
			case vk2::PhysicalDeviceType::CPU:
				rating += 100;
			break;
			default:
			break;
		}
		// Slight preference for NV/AMD cards, 250 points.
		switch(info.vendor)
		{
			case vk2::PhysicalDeviceVendor::Nvidia:
			[[fallthrough]];
			case vk2::PhysicalDeviceVendor::AMD:
				rating += 250;
			break;
			default: break;
		}
		unsigned int max_vram_size = 0;
		for(std::size_t i = 0; i < info.internal.memory.memoryHeapCount; i++)
		{
			if(info.internal.memory.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
			{
				max_vram_size = std::max(max_vram_size, static_cast<unsigned int>(info.internal.memory.memoryHeaps[i].size));
			}
		}
		// Each 5MiB of VRAM in the biggest device-local heap adds an extra point to the score.
		// This means an Integrated GPU could be taken over a Discrete GPU if it has 7.5GiB more VRAM, which is not happening anytime soon.
		rating += max_vram_size / (1024 * 1024 * 5);
		// If the device supports more than 64 buffer resources, add 500 rating.
		if(info.internal.limits.maxPerStageDescriptorStorageBuffers > 64)
		{
			rating += 500;
		}
		// If the device supports more than 64 image resources, add 500 rating.
		if(info.internal.limits.maxPerStageDescriptorSampledImages > 64)
		{
			rating += 500;
		}
		return rating;
	}

//--------------------------------------------------------------------------------------------------

	device_vulkan_base::device_vulkan_base():
	ldev([]()
	{
		vk2::PhysicalDeviceList pdevs = vk2::get_all_devices(vk2::get());
		tz::assert(!pdevs.empty(), "Could not locate any physical devices at all. Your machine either needs a valid GPU, CPU or a virtualised device acting as the former. Please ensure your machine meets minimum system requirements.");
		vk2::PhysicalDevice pdev = *std::max_element(pdevs.begin(), pdevs.end(),
		[](const vk2::PhysicalDevice& a, const vk2::PhysicalDevice& b)
		{
			return rate_device(a) < rate_device(b);
		});
		tz::report("Vulkan device: Out of %zu device%s, chose \"%s\" because it had the highest rating (%u)", pdevs.length(), pdevs.length() == 1 ? "" : "s", pdev.get_info().name.c_str(), rate_device(pdev));

		// TODO: Remove when we can get testing on devices that aren't NV.
		#if TZ_DEBUG
			if(pdev.get_info().vendor != vk2::PhysicalDeviceVendor::Nvidia)
			{
				const char* vendor_name = "<Unknown, sorry>";
				switch(pdev.get_info().vendor)
				{
					default:
					break;
					case vk2::PhysicalDeviceVendor::AMD:
						vendor_name = "AMD";
					break;
					case vk2::PhysicalDeviceVendor::Intel:
						vendor_name = "Intel";
					break;
				}
				tz::report("Developer: Warning: The selected device is not an Nvidia device. Non-nvidia devices are not tested at present, so if you do run into any issues please note on the bug report that your vendor is %s", vendor_name);
			}
		#endif

		vk2::DeviceExtensionList dev_exts = {};
		const vk2::DeviceFeatureField dev_feats =
		{
			vk2::DeviceFeature::DrawIndirectCount,
			vk2::DeviceFeature::MultiDrawIndirect,
			vk2::DeviceFeature::ShaderDrawParameters,
			vk2::DeviceFeature::BindlessDescriptors,
			vk2::DeviceFeature::ColourBlendLogicalOperations,
			vk2::DeviceFeature::NonSolidFillRasteriser,
			vk2::DeviceFeature::TessellationShaders,
			vk2::DeviceFeature::VertexPipelineResourceWrite,
			vk2::DeviceFeature::FragmentShaderResourceWrite,
			vk2::DeviceFeature::TimelineSemaphores,
			vk2::DeviceFeature::DynamicRendering
		};
		tz::assert(pdev.get_supported_features().contains(dev_feats), "One or both of DeviceFeatures 'BindlessDescriptors' and 'ColourBlendLogicalOperations' are not supported by this machine/driver. Please ensure your machine meets the system requirements.");
		dev_exts = {vk2::DeviceExtension::Swapchain};
		#if TZ_DEBUG
			dev_exts |= vk2::DeviceExtension::ShaderDebugPrint;
		#endif
		tz::assert(pdev.get_supported_extensions().contains(dev_exts), "One or more of the %zu required DeviceExtensions are not supported by this machine/driver. Please ensure your machine meets the system requirements.", dev_exts.count());
		return vk2::LogicalDevice
		{{
			.physical_device = pdev,
			.extensions = dev_exts,
			.features = dev_feats
		}};	
	}())
	{

	}

	const vk2::LogicalDevice& device_vulkan_base::vk_get_logical_device() const
	{
		return this->ldev;
	}

	vk2::LogicalDevice& device_vulkan_base::vk_get_logical_device()
	{
		return this->ldev;
	}

//--------------------------------------------------------------------------------------------------

	device_vulkan2::device_vulkan2():
	device_vulkan_base(),
	device_window(this->vk_get_logical_device()),
	device_render_sync(this->vk_get_logical_device(), this->render_graph().timeline),
	device_descriptor_pool(this->vk_get_logical_device()),
	device_command_pool(this->vk_get_logical_device())
	{

	}

	tz::gl::renderer_handle device_vulkan2::create_renderer(const tz::gl::renderer_info& rinfo)
	{
		return device_common<renderer_vulkan2>::emplace_renderer(rinfo);
	}
}

#endif // TZ_VULKAN
