#if TZ_VULKAN
#include "tz/gl/impl/vulkan/device.hpp"
#include "tz/core/profile.hpp"
#include "tz/core/debug.hpp"
#include "tz/dbgui/dbgui.hpp"
#include "tz/gl/impl/vulkan/renderer.hpp"
#include "tz/gl/impl/vulkan/convert.hpp"
#include "tz/gl/impl/vulkan/detail/extensions.hpp"
#include "tz/gl/impl/vulkan/detail/hardware/physical_device.hpp"
#include "tz/gl/impl/vulkan/detail/image_format.hpp"
#include <variant>

namespace tz::gl
{

	unsigned int rate_physical_device(const vk2::PhysicalDevice device)
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

	DeviceWindowVulkan::DeviceWindowVulkan(const vk2::LogicalDevice& device):
	DeviceWindowVulkan()
	{
		TZ_PROFZONE("DeviceWindowVulkan Create", 0xFFAAAA00);
		const vk2::VulkanInstance& instance = device.get_hardware().get_instance();
		tz::assert(instance.has_surface(), "DeviceWindowVulkan provided a VulkanInstance which is not headless, but doesn't have a WindowSurface attached. Please submit a bug report.");

		// Create a swapchain.
		// Ideally we want mailbox present mode, but that may not be available.
		vk2::SurfacePresentMode present_mode = vk2::SurfacePresentMode::Mailbox;
		if(!device.get_hardware().get_supported_surface_present_modes().contains(present_mode))
		{
			present_mode = device.get_hardware().get_supported_surface_present_modes().front();
		}
		std::uint32_t swapchain_img_min = device.get_hardware().get_surface_capabilities().min_image_count;
		std::uint32_t swapchain_img_max = device.get_hardware().get_surface_capabilities().max_image_count;
		std::uint32_t swapchain_img_count = std::clamp<std::uint32_t>(3u, swapchain_img_min, swapchain_img_max);
		swapchain_img_count = std::min<std::uint32_t>(swapchain_img_count, 3u);
		this->window_buf = vk2::Swapchain
		{{
			.device = &device,
			.swapchain_image_count_minimum = swapchain_img_count,
			.format = device.get_hardware().get_supported_surface_formats().front(),
			.present_mode = present_mode
		}};
		this->window_dims_cache = tz::window().get_dimensions();
		this->set_swapchain_images_debug_name();

		this->make_depth_image();
	}

	DeviceWindowVulkan::DeviceWindowVulkan(DeviceWindowVulkan&& move):
	window_buf(vk2::Swapchain::null())
	{
		*this = std::move(move);
	}

	DeviceWindowVulkan::~DeviceWindowVulkan()
	{
	}

	DeviceWindowVulkan& DeviceWindowVulkan::operator=(DeviceWindowVulkan&& rhs)
	{
		std::swap(this->window_buf, rhs.window_buf);
		return *this;
	}

	bool DeviceWindowVulkan::valid() const
	{
		return !this->get_swapchain().is_null();
	}

	const vk2::Swapchain& DeviceWindowVulkan::get_swapchain() const
	{
		return this->window_buf;
	}

	vk2::Swapchain& DeviceWindowVulkan::get_swapchain()
	{
		return this->window_buf;
	}

	tz::vec2ui DeviceWindowVulkan::get_dimensions() const
	{
		return this->get_swapchain().get_dimensions();
	}

	vk2::image_format DeviceWindowVulkan::get_format() const
	{
		return this->get_swapchain().get_image_format();
	}

	vk2::Swapchain::ImageAcquisitionResult DeviceWindowVulkan::get_unused_image(const vk2::Swapchain::ImageAcquisition& acquire)
	{
		if(!this->recent_acquire.has_value())
		{
			this->recent_acquire = this->get_swapchain().acquire_image(acquire);
		}
		return this->recent_acquire.value();
	}

	bool DeviceWindowVulkan::has_unused_image() const
	{
		return this->recent_acquire.has_value();
	}

	void DeviceWindowVulkan::mark_image_used()
	{
		this->recent_acquire = std::nullopt;
	}

	std::span<vk2::Image> DeviceWindowVulkan::get_output_images()
	{
		return this->get_swapchain().get_images();
	}

	vk2::Image& DeviceWindowVulkan::get_depth_image()
	{
		return this->depth_image;
	}

	bool DeviceWindowVulkan::request_refresh()
	{
		auto dims = tz::window().get_dimensions();
		if(this->window_dims_cache != dims)
		{
			this->on_resize(dims);
			this->window_dims_cache = dims;
			return true;
		}
		return false;
	}

	void DeviceWindowVulkan::set_swapchain_images_debug_name()
	{
		#if TZ_DEBUG
			for(std::size_t i = 0; i < this->window_buf.get_images().size(); i++)
			{
				this->window_buf.get_images()[i].debug_set_name("device Swapchain Image " + std::to_string (i));
			}
		#endif
	}


	void DeviceWindowVulkan::make_depth_image()
	{
		this->depth_image = vk2::Image
		{vk2::image_info{
			.device = &this->get_swapchain().get_device(),
			.format = vk2::image_format::Depth32_SFloat,
			.dimensions = this->get_swapchain().get_dimensions(),
			.usage = {vk2::ImageUsage::DepthStencilAttachment},
			.residency = vk2::MemoryResidency::GPU
		}};
		this->depth_image.debug_set_name("device Depth Image");
	}

	void DeviceWindowVulkan::on_resize(tz::vec2ui dims)
	{
		TZ_PROFZONE("DeviceWindowVulkan OnResize", 0xFFAAAA00);
		if(dims[0] == 0 || dims[0] == 0)
		{
			return;
		}
		// Assume we have a head, headless no support for resizeable output yet.
		vk2::Swapchain& old_swapchain = this->get_swapchain();
		if(old_swapchain.get_dimensions() == dims)
		{
			// Dimensions didn't actually change. Early out and don't bother telling the renderers.
			return;
		}
		if(dims[0] == 0 || dims[1] == 0)
		{
			// Cannot create a swapchain, We just hold off on rendering.
			tz::wsi::wait_for_event();
			return;
		}
		vk2::Swapchain new_swapchain
		{{
			.device = &old_swapchain.get_device(),
			.swapchain_image_count_minimum = static_cast<std::uint32_t>(old_swapchain.get_image_views().size()),
			.format = old_swapchain.get_image_format(),
			.present_mode = old_swapchain.get_present_mode(),
			.old_swapchain = &old_swapchain
		}};
		std::swap(old_swapchain, new_swapchain);
		this->old_depth_image = std::move(this->depth_image);
		this->make_depth_image();
		this->set_swapchain_images_debug_name();
	}

//--------------------------------------------------------------------------------------------------
	DeviceRenderSchedulerVulkan::DeviceRenderSchedulerVulkan(const vk2::LogicalDevice& ldev, std::size_t frame_in_flight_count):
	ldev(&ldev),
	image_available(),
	render_work_done(),
	frame_work()
	{
		this->image_available.reserve(frame_in_flight_count);
		this->render_work_done.reserve(frame_in_flight_count);
		this->frame_work.reserve(frame_in_flight_count);
		for(std::size_t i = 0; i < frame_in_flight_count; i++)
		{
			auto& iasev = this->image_available.emplace_back(ldev);
			iasev.debug_set_name("device Image Semaphore " + std::to_string(i));
			auto& rwsev = this->render_work_done.emplace_back(ldev);
			rwsev.debug_set_name("device Render Semaphore " + std::to_string(i));
			auto& fence = this->frame_work.emplace_back(vk2::FenceInfo
			{
				.device = &ldev,
				.initially_signalled = true
			});
			fence.debug_set_name("device Frame Fence " + std::to_string(i));
		}
	}

	void DeviceRenderSchedulerVulkan::notify_renderer_added()
	{
		this->renderer_timelines.emplace_back(*this->ldev, 0);
	}

	void DeviceRenderSchedulerVulkan::notify_renderer_removed(std::size_t renderer_id)
	{
		this->renderer_timelines.erase(this->renderer_timelines.begin() + renderer_id);
	}

	std::span<const vk2::BinarySemaphore> DeviceRenderSchedulerVulkan::get_image_signals() const
	{
		return this->image_available;
	}

	std::span<vk2::BinarySemaphore> DeviceRenderSchedulerVulkan::get_image_signals()
	{
		return this->image_available;
	}

	std::span<const vk2::BinarySemaphore> DeviceRenderSchedulerVulkan::get_render_work_signals() const
	{
		return this->render_work_done;
	}

	std::span<const vk2::Fence> DeviceRenderSchedulerVulkan::get_frame_fences() const
	{
		return this->frame_work;
	}

	std::span<vk2::Fence> DeviceRenderSchedulerVulkan::get_frame_fences()
	{
		return this->frame_work;
	}

	std::span<const vk2::TimelineSemaphore> DeviceRenderSchedulerVulkan::get_renderer_timelines() const
	{
		return this->renderer_timelines;
	}

	std::span<vk2::TimelineSemaphore> DeviceRenderSchedulerVulkan::get_renderer_timelines()
	{
		return this->renderer_timelines;
	}

	void DeviceRenderSchedulerVulkan::wait_frame_work_complete() const
	{
		for(const vk2::Fence& fence : this->frame_work)
		{
			fence.wait_until_signalled();
		}
	}

	void DeviceRenderSchedulerVulkan::new_frame()
	{
		// our frame_id is some value. we need to make sure the frame work at that id (from last time we had this id) is done. we wait on that.
		this->frame_work[this->frame_id].wait_until_signalled();
		// unsignal it. we're gonna use it again.
		this->frame_work[this->frame_id].unsignal();
		this->frame_id = (this->frame_id + 1) % this->frame_work.size();
	}

	void DeviceRenderSchedulerVulkan::clear_renderers()
	{
		this->renderer_timelines.clear();
	}

//--------------------------------------------------------------------------------------------------

	device_vulkan::device_vulkan():
	device_vulkan(vk2::get())
	{

	}

	device_vulkan::device_vulkan(const vk2::VulkanInstance& instance):
	device(device_vulkan::make_device(instance)),
	window_storage(this->device),
	scheduler(this->device, this->window_storage.get_output_images().size())
	{
		TZ_PROFZONE("Vulkan Frontend - device_vulkan Create", 0xFFAAAA00);
	}

	device_vulkan::~device_vulkan()
	{
		this->scheduler.wait_frame_work_complete();
		this->scheduler.clear_renderers();
		device_common<renderer_vulkan>::internal_clear();
	}

	tz::gl::renderer_handle device_vulkan::create_renderer(const renderer_info& info)
	{
		TZ_PROFZONE("Vulkan Frontend - renderer Create (via device_vulkan)", 0xFFAAAA00);
		this->scheduler.notify_renderer_added();
		return device_common<renderer_vulkan>::emplace_renderer(info);
	}

	void device_vulkan::destroy_renderer(tz::gl::renderer_handle handle)
	{
		this->scheduler.notify_renderer_removed(static_cast<std::size_t>(static_cast<tz::hanval>(handle)));
		device_common<renderer_vulkan>::destroy_renderer(handle);
	}

	image_format device_vulkan::get_window_format() const
	{
		return from_vk2(this->window_storage.get_format());
	}

	void device_vulkan::dbgui()
	{
		tz::gl::common_device_dbgui(*this);
	}

	void device_vulkan::begin_frame()
	{
		this->scheduler.new_frame();
	}

	void device_vulkan::end_frame()
	{
	}

	const DeviceWindowVulkan& device_vulkan::get_device_window() const
	{
		return this->window_storage;
	}

	DeviceWindowVulkan& device_vulkan::get_device_window()
	{
		return this->window_storage;
	}

	const DeviceRenderSchedulerVulkan& device_vulkan::get_render_scheduler() const
	{
		return this->scheduler;
	}

	DeviceRenderSchedulerVulkan& device_vulkan::get_render_scheduler()
	{
		return this->scheduler;
	}

	const vk2::LogicalDevice& device_vulkan::vk_get_logical_device() const
	{
		return this->device;
	}

	vk2::LogicalDevice& device_vulkan::vk_get_logical_device()
	{
		return this->device;
	}

	/*static*/vk2::LogicalDevice device_vulkan::make_device(const vk2::VulkanInstance& instance)
	{
		// First, create a LogicalDevice.
		// TODO: Don't just choose a device at random.
		vk2::PhysicalDeviceList pdevs = vk2::get_all_devices(instance);
		tz::assert(!pdevs.empty(), "Could not locate any physical devices at all. Your machine either needs a valid GPU, CPU or a virtualised device acting as the former. Please ensure your machine meets minimum system requirements.");
		vk2::PhysicalDevice pdev = *std::max_element(pdevs.begin(), pdevs.end(),
		[](const vk2::PhysicalDevice& a, const vk2::PhysicalDevice& b)
		{
			return rate_physical_device(a) < rate_physical_device(b);
		});
		tz::report("Vulkan device: Out of %zu device%s, chose \"%s\" because it had the highest rating (%u)", pdevs.length(), pdevs.length() == 1 ? "" : "s", pdev.get_info().name.c_str(), rate_physical_device(pdev));

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
		return
		{{
			.physical_device = pdev,
			.extensions = dev_exts,
			.features = dev_feats
		}};
		
	}
}

#endif // TZ_VULKAN
