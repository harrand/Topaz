#if TZ_VULKAN
#include "tz/gl/impl/vulkan/device2.hpp"
#include "tz/gl/impl/common/device.dbgui.hpp"
#include "tz/gl/impl/vulkan/detail/semaphore.hpp"
#include "tz/gl/impl/vulkan/detail/fence.hpp"
#include "tz/gl/impl/vulkan/detail/tz_vulkan.hpp"
#include "tz/gl/impl/vulkan/convert.hpp"
#include "tz/core/profile.hpp"
#include <algorithm>

#define TZ_VK_RENDER_DEBUG_LOGGING 0

#if TZ_VK_RENDER_DEBUG_LOGGING
#include <iostream>
#endif


namespace tz::gl
{

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
		TZ_PROFZONE("device_vulkan_base - initialise", 0xFFAAAA00);
		vk2::PhysicalDeviceList pdevs = vk2::get_all_devices(vk2::get());
		tz::assert(!pdevs.empty(), "Could not locate any physical devices at all. Your machine either needs a valid GPU, CPU or a virtualised device acting as the former. Please ensure your machine meets minimum system requirements.");
		vk2::PhysicalDevice pdev = *std::max_element(pdevs.begin(), pdevs.end(),
		[](const vk2::PhysicalDevice& a, const vk2::PhysicalDevice& b)
		{
			return rate_device(a) < rate_device(b);
		});

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

	std::size_t device_vulkan_base::get_rid(unsigned int fingerprint) const
	{
		return this->fingerprint_to_renderer_id.at(fingerprint);
	}

	void device_vulkan_base::touch_renderer_id(unsigned int fingerprint, std::size_t renderer_id)
	{
		this->fingerprint_to_renderer_id.emplace(fingerprint, renderer_id);
	}
	

//--------------------------------------------------------------------------------------------------

	device_window::device_window():
	device_vulkan_base()
	{
		// choose initial settings and create swapchain + depth.
		TZ_PROFZONE("device_window - initialise", 0xFFAA0000);
		const vk2::PhysicalDevice& hardware = this->ldev.get_hardware();
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
			.device = &this->ldev,
			.swapchain_image_count_minimum = swapchain_image_count,
			.format = chosen_format,
			.present_mode = chosen_present_mode
		}};
		this->dimensions_cache = tz::window().get_dimensions();
		this->make_depth_image();
		this->initialise_image_semaphores();
		this->debug_annotate_resources();
	}

	tz::gl::image_format device_window::get_window_format() const
	{
		return tz::gl::from_vk2(this->get_swapchain().get_image_format());
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

	const vk2::BinarySemaphore& device_window::acquire_image(const vk2::Fence* signal_fence)
	{
		TZ_PROFZONE("device_window - acquire image", 0xFFAA0000);
		// do an acquire, choose a semaphore for it to signal, and then return that to the caller.
		// the caller is expected to use that sem as a wait semaphore for their render work. they can't render into the device window until the semaphore is signalled.
		// additionally the fence can be provided to do a cpu wait. not sure why you'd wanna do that tho.
		const vk2::BinarySemaphore& sem = this->get_image_semaphores()[device_vulkan_base::frame_id];
		if(!this->recent_acquire.has_value())
		{
			this->recent_acquire = this->swapchain.acquire_image(vk2::Swapchain::ImageAcquisition
			{
				.signal_semaphore = &sem,
				.signal_fence = signal_fence
			});
			switch(this->recent_acquire->type)
			{
				case vk2::Swapchain::ImageAcquisitionResult::AcquisitionResultType::AcquireSuccess:

				break;
				case vk2::Swapchain::ImageAcquisitionResult::AcquisitionResultType::Suboptimal:

				break;
				case vk2::Swapchain::ImageAcquisitionResult::AcquisitionResultType::ErrorOutOfDate:
					this->vk_notify_resize();
					return this->acquire_image(signal_fence);
				break;
				case vk2::Swapchain::ImageAcquisitionResult::AcquisitionResultType::ErrorSurfaceLost:
					tz::error("Failed to acquire swapchain image because surface was lost. Please submit a bug report.");
				break;
				case vk2::Swapchain::ImageAcquisitionResult::AcquisitionResultType::ErrorUnknown:
					tz::error("Failed to acquire swapchain image, but couldn't determine why. Please submit a bug report.");
				break;
				default: tz::error(); break;
			}
		}
		return sem;
	}

	std::size_t device_window::get_image_index() const
	{
		tz::assert(this->recent_acquire.has_value());
		return this->recent_acquire->image_index;
	}

	std::span<vk2::BinarySemaphore> device_window::get_image_semaphores()
	{
		return this->image_semaphores;
	}

	const vk2::Swapchain::ImageAcquisitionResult* device_window::get_recent_acquire() const
	{
		return this->recent_acquire.has_value() ? &this->recent_acquire.value() : nullptr;
	}

	void device_window::vk_acquire_done()
	{
		this->recent_acquire = std::nullopt;
	}

	void device_window::vk_notify_resize()
	{
		TZ_PROFZONE("device_window - resize notify", 0xFFAA0000);
		if(tz::window().get_dimensions() == tz::vec2ui::zero())
		{
			TZ_PROFZONE("device_window - wait on minimise", 0xFFAA0000);
			tz::wsi::wait_for_event();
			return;
		}
		if(this->dimensions_cache != tz::window().get_dimensions())
		{
			device_vulkan_base::vk_get_logical_device().wait_until_idle();
			this->swapchain.refresh();
			this->make_depth_image();
			this->dimensions_cache = tz::window().get_dimensions();
		}
	}

	void device_window::make_depth_image()
	{
		TZ_PROFZONE("device_window - make depth image", 0xFFAA0000);
		tz::assert(!this->swapchain.is_null());
		this->device_depth =
		{{
			.device = &this->ldev,
			.format = vk2::image_format::Depth32_SFloat,
			.dimensions = this->swapchain.get_dimensions(),
			.usage = {vk2::ImageUsage::DepthStencilAttachment},
			.residency = vk2::MemoryResidency::GPU
		}};
	}

	void device_window::initialise_image_semaphores()
	{
		for(std::size_t i = 0; i < this->get_swapchain().get_images().size(); i++)
		{
			this->image_semaphores.emplace_back(this->ldev);
		}
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

	device_render_sync::device_render_sync(device_common<renderer_vulkan2>& devcom):
	device_window(),
	sched(devcom.render_graph())
	{
		for(std::size_t i = 0; i < device_window::get_swapchain().get_images().size(); i++)
		{
			this->dependency_syncs.emplace_back(ldev, 0);
			this->frame_syncs.emplace_back(ldev, 0);
		}
	}

	void device_render_sync::vk_frame_wait(unsigned int fingerprint)
	{
		TZ_PROFZONE("device_render_sync - frame wait", 0xFFAA0000);
		if(this->get_timeline().empty())
		{
			tz::report("Detected wait on frame boundary, even though the timeline was never populated. You may be about to run into scheduling errors");
		}
		// if the first renderer of the frame needs to go, we need to make sure it waits on the frame fence.
		if(device_vulkan_base::frame_counter > 0 && this->get_timeline().front() == device_vulkan_base::get_rid(fingerprint))
		{
			// we are waiting for the current work to finish.
			this->frame_syncs[device_vulkan_base::old_frame_id].wait_for(device_vulkan_base::frame_counter);
		}
	}

	void device_render_sync::vk_skip_renderer(unsigned int fingerprint)
	{
		// a renderer isn't going to submit any commands this frame, but we want to pretend as if it did in terms of sync.
		// to do this, we instantly set the dependency timeline semaphore to what it needs to be.
		vk2::TimelineSemaphore& dep_sem = device_render_sync::get_dependency_sync_objects()[device_vulkan_base::frame_id];
		std::size_t eid = device_vulkan_base::get_rid(fingerprint);
		unsigned int our_rank = this->get_schedule().chronological_rank_eid(eid);
		unsigned int signal_target = our_rank + 1 + device_vulkan_base::global_timeline;
		if(signal_target > device_vulkan_base::max_signal_rank_this_frame && signal_target > dep_sem.get_value())
		{
			dep_sem.signal(signal_target);
		}
		if(our_rank == this->get_schedule().max_chronological_rank() && !device_vulkan_base::frame_signal_sent_this_frame)
		{
			device_vulkan_base::frame_signal_sent_this_frame = true;
			device_render_sync::get_frame_sync_objects()[device_vulkan_base::frame_id].signal(device_vulkan_base::frame_counter + 1);
		}
	}

	const tz::gl::schedule& device_render_sync::get_schedule() const
	{
		return this->sched;
	}

	const tz::gl::timeline_t& device_render_sync::get_timeline() const
	{
		return this->get_schedule().timeline;
	}

	std::vector<const vk2::Semaphore*> device_render_sync::vk_get_dependency_waits(unsigned int fingerprint)
	{
		(void)fingerprint;
		return {};
	}

	std::vector<const vk2::Semaphore*> device_render_sync::vk_get_dependency_signals(unsigned int fingerprint)
	{
		(void)fingerprint;
		return {};
	}

	std::span<const vk2::TimelineSemaphore> device_render_sync::get_frame_sync_objects() const
	{
		return this->frame_syncs;
	}

	std::span<vk2::TimelineSemaphore> device_render_sync::get_frame_sync_objects()
	{
		return this->frame_syncs;
	}

	std::span<vk2::TimelineSemaphore> device_render_sync::get_dependency_sync_objects()
	{
		return this->dependency_syncs;
	}

	std::uint64_t device_render_sync::get_sync_id(std::size_t renderer_id) const
	{
		const tz::gl::schedule& sch = device_render_sync::get_schedule();
		std::uint64_t tl_val = 0;
		for(tz::gl::eid_t evt : sch.get_dependencies(renderer_id))
		{
			tl_val = std::max(tl_val, get_sync_id(evt));
		}
		if(sch.get_dependencies(renderer_id).size())
		{
			tl_val++;
		}
		return tl_val;
	}

	std::uint64_t device_render_sync::get_renderer_sync_id(unsigned int fingerprint) const
	{
		return get_sync_id(device_vulkan_base::get_rid(fingerprint));
	}

//--------------------------------------------------------------------------------------------------

	device_descriptor_pool::device_descriptor_pool(device_common<renderer_vulkan2>& devcom):
	device_render_sync(devcom)
	{
		this->another_pool();
	}

	vk2::DescriptorPool::UpdateRequest device_descriptor_pool::vk_make_update_request(unsigned int fingerprint)
	{
		vk2::DescriptorPool& pool = this->get_pool(fingerprint);
		return pool.make_update_request();
	}

	vk2::DescriptorPool::AllocationResult device_descriptor_pool::vk_allocate_sets(const vk2::DescriptorPool::Allocation& alloc, unsigned int fingerprint)
	{
		TZ_PROFZONE("device_descriptor_pool - allocate sets", 0xFFAA0000);
		return this->impl_allocate_sets(alloc, fingerprint, 0);
	}

	void device_descriptor_pool::vk_update_sets(vk2::DescriptorPool::UpdateRequest update, unsigned int fingerprint)
	{
		TZ_PROFZONE("device_descriptor_pool - update sets", 0xFFAA0000);
		this->get_pool(fingerprint).update_sets(update);
	}

	vk2::DescriptorPool& device_descriptor_pool::get_pool(unsigned int fingerprint)
	{
		tz::assert(this->fingerprint_to_pool_id.find(fingerprint) != this->fingerprint_to_pool_id.end(), "A renderer requested to write descriptor sets with fingerprint %u, but no such fingerprint was ever used to allocate sets.", fingerprint);
		return this->pools[this->fingerprint_to_pool_id.at(fingerprint)];
	}

	vk2::DescriptorPool::AllocationResult device_descriptor_pool::impl_allocate_sets(const vk2::DescriptorPool::Allocation& alloc, unsigned int fingerprint, unsigned int attempt)
	{
		TZ_PROFZONE("device_descriptor_pool - set alloc attempt", 0xFFAA0000);
		// allocate using the most recently created descriptor pool
		// if allocation fails due to lack of memory, create a new descriptor pool and try again.
		// if it fails too many times in a row, we're gonna error out.
		constexpr unsigned int failure_attempt = 8;
		tz::assert(attempt < failure_attempt, "Failed to allocate descriptor sets for a renderer after %u attempts. Please submit a bug report", failure_attempt);

		vk2::DescriptorPool::AllocationResult ret = this->pools.back().allocate_sets(alloc);
		using ART = vk2::DescriptorPool::AllocationResult::AllocationResultType;
		if(ret.type == ART::AllocationSuccess)
		{
			this->fingerprint_to_pool_id[fingerprint] = this->pools.size() - 1;
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
		TZ_PROFZONE("device_descriptor_pool - another pool", 0xFFAA0000);
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
		pool_lims.supports_update_after_bind = true;
		this->pools.emplace_back(vk2::DescriptorPoolInfo
		{
			.limits = pool_lims,
			.logical_device = &this->ldev
		});
	}

//--------------------------------------------------------------------------------------------------

	device_command_pool::device_command_pool(device_common<renderer_vulkan2>& devcom):
	device_descriptor_pool(devcom)
	{
		TZ_PROFZONE("device_command_pool - initialise", 0xFFAA0000);
		this->graphics_queue = this->ldev.get_hardware_queue
		({
			.field = {vk2::QueueFamilyType::graphics},
	   		.present_support = false
   		});

		this->graphics_present_queue = this->ldev.get_hardware_queue
		({
			.field = {vk2::QueueFamilyType::graphics},
	   		.present_support = true
   		});

		this->compute_queue = this->ldev.get_hardware_queue
		({
			.field = {vk2::QueueFamilyType::compute},
	   		.present_support = false
   		});

		this->graphics_commands = {{.queue = this->graphics_queue}};
		this->graphics_present_commands = {{.queue = this->graphics_present_queue}};
		this->compute_commands = {{.queue = this->compute_queue}};
	}

	vk2::CommandPool::AllocationResult device_command_pool::vk_allocate_commands(const vk2::CommandPool::Allocation& alloc, unsigned int fingerprint)
	{
		TZ_PROFZONE("device_command_pool - allocate commands", 0xFFAA0000);
		tz::assert(alloc.buffer_count > 0, "it's illegal to do a command buffer allocation of size zero. please submit a bug report.");
		return this->impl_allocate_commands(alloc, fingerprint, 0);
	}

	void device_command_pool::vk_free_commands(unsigned int fingerprint, std::size_t allocation_id, std::span<vk2::CommandBuffer> command_buffers)
	{
		TZ_PROFZONE("device_command_pool - free commands", 0xFFAA0000);
		tz::assert(this->fingerprint_allocation_history.find(fingerprint) != this->fingerprint_allocation_history.end(), "cannot free commands because there is no recorded history for fingerprint %u", fingerprint);
		auto& allocations = this->fingerprint_allocation_history.at(fingerprint);
		tz::assert(!allocations.empty(), "attempted to free command buffers of allocation-id %zu for fingerprint %u, but there were no allocations -- either a zero-allocation occurred (wouldve asserted earlier), or this has already been freed.");
		tz::assert(allocations.size() > allocation_id, "allocation id %zu for fingerprint %u is invalid - only %zu allocations are in the history for this fingerprint", allocation_id, fingerprint, allocations.size());
		tz::assert(allocations[allocation_id].buffer_count == command_buffers.size(), "passed span for command buffers in free (size %zu) does not match the number of buffers that were allocated (%zu). the span does not match. logic error. please submit a bug report.", command_buffers.size(), allocations[allocation_id].buffer_count);
		
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
		TZ_PROFZONE("device_command_pool - submit & run & block", 0xFFAA0000);
		vk2::Fence temp_fence
		{{
			.device = &this->ldev
		}};

		#if TZ_DEBUG
			std::size_t debug_real_alloc_length = this->fingerprint_allocation_history[fingerprint].size();
			tz::assert(debug_real_alloc_length > buffer_id, "attempted to submit & run scratch command buffer id %zu at (fingerprint:allocid) %u:%zu. there are only %zu buffers in this allocation. please submit a bug report.", buffer_id, fingerprint, allocation_id, debug_real_alloc_length);
		#else
			(void)buffer_id;
			(void)allocation_id;
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

	void device_command_pool::vk_submit_command(unsigned int fingerprint, std::size_t allocation_id, std::size_t buffer_id, std::span<const vk2::CommandBuffer> cmdbufs, const tz::basic_list<const vk2::BinarySemaphore*>& extra_waits, const tz::basic_list<const vk2::BinarySemaphore*>& extra_signals, vk2::Fence* signal_fence)
	{
		TZ_PROFZONE("device_command_pool - submit command buffer", 0xFFAA0000);
		#if TZ_DEBUG
			std::size_t debug_real_alloc_length = this->fingerprint_allocation_history[fingerprint].size();
			tz::assert(debug_real_alloc_length > allocation_id, "attempted to submit work command buffer id %zu at (fingerprint:allocid) %u:%zu. there are only %zu buffers in this allocation. please submit a bug report.", allocation_id, fingerprint, allocation_id, debug_real_alloc_length);
			tz::assert(this->fingerprint_allocation_history[fingerprint][allocation_id].buffer_count > buffer_id, "attempted to submit work command buffer id %zu, but there are only %zu buffers in this allocation. logic error. please submit a bug report.", buffer_id, this->fingerprint_allocation_history[fingerprint][allocation_id].buffer_count);
		#else
			(void)buffer_id;
			(void)allocation_id;
		#endif // TZ_DEBUG
		vk2::hardware::Queue* q = this->get_original_queue(this->fingerprint_alloc_types[fingerprint]);	
		vk2::TimelineSemaphore& dep_sem = device_render_sync::get_dependency_sync_objects()[device_vulkan_base::frame_id];
		tz::assert(!dep_sem.is_null());

		vk2::hardware::Queue::SubmitInfo submit;
		tz::basic_list<vk2::hardware::Queue::SubmitInfo::WaitInfo> waits = {};
		tz::basic_list<vk2::hardware::Queue::SubmitInfo::SignalInfo> signals = {};

		// does this have a set of dependencies? if so we wait on it.
		std::size_t eid = device_vulkan_base::get_rid(fingerprint);
		#if TZ_VK_RENDER_DEBUG_LOGGING
			std::cout << "===========" << eid << "===========\n";
		#endif
		auto maybe_wait_rank = this->get_schedule().get_wait_rank(eid);
		if(maybe_wait_rank.has_value())
		{
			const std::size_t wait_value = maybe_wait_rank.value() + device_vulkan_base::global_timeline + 1;
			waits.add({.wait_semaphore = &dep_sem, .wait_stage = vk2::PipelineStage::AllCommands, .timeline = wait_value});
			#if TZ_VK_RENDER_DEBUG_LOGGING
				std::cout << "wait dep: " << wait_value << "\n";
			#endif
		}

		// also add the waits someone else gave us. just assume its necessary binary semaphores.
		for(const vk2::Semaphore* wait : extra_waits)
		{
			waits.add({.wait_semaphore = wait, .wait_stage = vk2::PipelineStage::AllCommands});
		}

		// so heres a problem. we could have multiple renderers with this same chronological rank. if so, we only signal once.
		// so we store a value telling us the max signal rank this frame.
		// if nobodys signalled our rank yet, then we send it.
		unsigned int our_rank = this->get_schedule().chronological_rank_eid(eid);
		unsigned int signal_target = our_rank + 1 + device_vulkan_base::global_timeline;
		if(signal_target > device_vulkan_base::max_signal_rank_this_frame)
		{
			device_vulkan_base::max_signal_rank_this_frame = signal_target;
			signals.add({.signal_semaphore = &dep_sem, .timeline = signal_target});
			#if TZ_VK_RENDER_DEBUG_LOGGING
				std::cout << "signal dep: " << signal_target << "\n";
			#endif
		}
		// also we might have some binary semaphores we need to signal to, make sure we send those.
		for(const vk2::Semaphore* signal : extra_signals)
		{
			signals.add({.signal_semaphore = signal});
		}

		// the last thing. we want to be able to wait on frames, or the first renderer of the next frame needs to be aware.
		// we should signal that magic semaphore if: we are the max rank, and it hasn't been signalled already yet.
		if(our_rank == this->get_schedule().max_chronological_rank() && !device_vulkan_base::frame_signal_sent_this_frame)
		{
			device_vulkan_base::frame_signal_sent_this_frame = true;
			signals.add({.signal_semaphore = &device_render_sync::get_frame_sync_objects()[device_vulkan_base::frame_id], .timeline = device_vulkan_base::frame_counter + 1});
			#if TZ_VK_RENDER_DEBUG_LOGGING
				std::cout << "signal frame: " << device_vulkan_base::frame_counter + 1 << "\n";
			#endif
		}

		tz::basic_list<const vk2::CommandBuffer*> buffers = {};
		for(const vk2::CommandBuffer& buf : cmdbufs)
		{
			buffers.add(&buf);
		}
		submit.command_buffers = buffers;
		submit.waits = waits;
		submit.signals = signals;
		submit.execution_complete_fence = signal_fence;
		q->submit(submit);
		#if TZ_VK_RENDER_DEBUG_LOGGING
			std::cout << "======================\n\n";
		#endif
	}

	vk2::hardware::Queue::PresentResult device_command_pool::present_image(unsigned int fingerprint, const tz::basic_list<const vk2::BinarySemaphore*>& wait_semaphores)
	{
		TZ_PROFZONE("device_command_pool - present image", 0xFFAA0000);
		tz::assert(device_window::get_recent_acquire() != nullptr, "Attempting to present image, but no image has been previously acquired. Logic error.");
		vk2::hardware::Queue* present_queue = get_original_queue(this->fingerprint_alloc_types.at(fingerprint));
		vk2::hardware::Queue::PresentResult res = present_queue->present
		({
			.wait_semaphores = wait_semaphores,
	   		.swapchain = &device_window::get_swapchain(),
			.swapchain_image_index = device_window::get_recent_acquire()->image_index
		});
		device_window::vk_acquire_done();
		return res;
	}


	vk2::CommandPool::AllocationResult device_command_pool::impl_allocate_commands(const vk2::CommandPool::Allocation& alloc, unsigned int fingerprint, unsigned int attempt)
	{
		TZ_PROFZONE("device_command_pool - command alloc attempt", 0xFFAA0000);
		// allocate using the most recently created descriptor pool
		// if allocation fails due to lack of memory, create a new descriptor pool and try again.
		// if it fails too many times in a row, we're gonna error out.
		constexpr unsigned int failure_attempt = 8;
		tz::assert(attempt < failure_attempt, "Failed to allocate command buffers for a renderer after %u attempts. Please submit a bug report", failure_attempt);
		vk2::CommandPool& pool = this->get_fitting_pool(this->fingerprint_alloc_types.at(fingerprint));

		vk2::CommandPool::AllocationResult ret = pool.allocate_buffers(alloc);
		using ART = vk2::CommandPool::AllocationResult::AllocationResultType;

		// we want to re-use an old allocation slot if we can that was freed. they will have command buffer count = 0
		std::optional<std::size_t> reuseable_alloc_id = std::nullopt;
		for(std::size_t allocid = 0; allocid < this->fingerprint_allocation_history[fingerprint].size(); allocid++)
		{
			if(this->fingerprint_allocation_history[fingerprint][allocid].buffer_count == 0)
			{
				reuseable_alloc_id = allocid;
				break;
			}
		}

		if(ret.type == ART::AllocationSuccess)
		{
			if(reuseable_alloc_id.has_value())
			{
				this->fingerprint_allocation_history[fingerprint][reuseable_alloc_id.value()] = alloc;
			}
			else
			{
				this->fingerprint_allocation_history[fingerprint].push_back(alloc);
			}
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

	device_vulkan2::device_vulkan2():
	device_command_pool(static_cast<device_common<tz::gl::renderer_vulkan2>&>(*this))
	{

	}

	device_vulkan2::~device_vulkan2()
	{
		TZ_PROFZONE("device_vulkan2 - destroy", 0xFFAAAA00);
		{
			TZ_PROFZONE("wait until idle", 0xFFAAAA00);
			device_vulkan_base::vk_get_logical_device().wait_until_idle();
		}
		device_common<renderer_vulkan2>::internal_clear();
	}

	tz::gl::renderer_handle device_vulkan2::create_renderer(const tz::gl::renderer_info& rinfo)
	{
		TZ_PROFZONE("device_vulkan2 - create renderer", 0xFFAA0000);
		tz::gl::renderer_handle rh = device_common<renderer_vulkan2>::emplace_renderer(rinfo);
		device_vulkan_base::touch_renderer_id(device_common<renderer_vulkan2>::get_renderer(rh).vk_get_uid(), device_common<renderer_vulkan2>::renderer_count() - 1);
		return rh;
	}

	void device_vulkan2::dbgui()
	{
		tz::gl::common_device_dbgui(*this);
	}

	void device_vulkan2::end_frame()
	{
		TZ_PROFZONE("device_vulkan2 - end frame", 0xFFAA0000);
		device_vulkan_base::global_timeline += device_common<renderer_vulkan2>::render_graph().max_chronological_rank() + 1;
		#if TZ_VK_RENDER_DEBUG_LOGGING
			std::cout << "end frame " << device_vulkan_base::frame_counter << ". global timeline = " << device_vulkan_base::global_timeline << "\n";
		#endif
		device_vulkan_base::frame_counter++;
		device_vulkan_base::old_frame_id = device_vulkan_base::frame_id;
		device_vulkan_base::frame_id = (device_vulkan_base::frame_id + 1) % device_window::get_swapchain().get_images().size();
		device_vulkan_base::max_signal_rank_this_frame = 0;
		tz::assert(device_vulkan_base::frame_signal_sent_this_frame);
		device_vulkan_base::frame_signal_sent_this_frame = false;
	}

	void device_vulkan2::full_wait() const
	{
		TZ_PROFZONE("device_vulkan2 - full wait", 0xFFAAAA00);
		device_vulkan_base::vk_get_logical_device().wait_until_idle();
	}

	void device_vulkan2::frame_wait() const
	{
		TZ_PROFZONE("device_vulkan2 - frame wait", 0xFFAAAA00);
		device_render_sync::get_frame_sync_objects()[device_vulkan_base::old_frame_id].wait_for(device_vulkan_base::frame_counter);
	}

}

#endif // TZ_VULKAN
