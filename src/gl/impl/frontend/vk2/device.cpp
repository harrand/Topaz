#if TZ_VULKAN
#include "core/profiling/zone.hpp"
#include "core/report.hpp"
#include "gl/impl/frontend/vk2/renderer.hpp"
#include "gl/impl/frontend/vk2/device.hpp"
#include "gl/impl/frontend/vk2/convert.hpp"
#include "gl/impl/backend/vk2/extensions.hpp"
#include "gl/impl/backend/vk2/hardware/physical_device.hpp"
#include "gl/impl/backend/vk2/image_format.hpp"
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
		return rating;
	}

//--------------------------------------------------------------------------------------------------

	DeviceWindowVulkan::DeviceWindowVulkan(const vk2::LogicalDevice& device):
	DeviceWindowVulkan()
	{
		TZ_PROFZONE("DeviceWindowVulkan Create", TZ_PROFCOL_YELLOW);
		const vk2::VulkanInstance& instance = device.get_hardware().get_instance();
		tz_assert(instance.has_surface(), "DeviceWindowVulkan provided a VulkanInstance which is not headless, but doesn't have a WindowSurface attached. Please submit a bug report.");

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
			.image_format = device.get_hardware().get_supported_surface_formats().front(),
			.present_mode = present_mode
		}};
		this->register_resize();
	}

	DeviceWindowVulkan::DeviceWindowVulkan(DeviceWindowVulkan&& move):
	window_buf(vk2::Swapchain::null())
	{
		*this = std::move(move);
	}

	DeviceWindowVulkan::~DeviceWindowVulkan()
	{
		this->unregister_resize();
	}

	DeviceWindowVulkan& DeviceWindowVulkan::operator=(DeviceWindowVulkan&& rhs)
	{
		std::swap(this->window_buf, rhs.window_buf);
		std::swap(this->on_resize_handle, rhs.on_resize_handle);
		this->reregister_resize();
		rhs.unregister_resize();
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

	tz::Vec2ui DeviceWindowVulkan::get_dimensions() const
	{
		return this->get_swapchain().get_dimensions();
	}

	vk2::ImageFormat DeviceWindowVulkan::get_format() const
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

	RendererResizeCallbackType& DeviceWindowVulkan::resize_callback()
	{
		return this->renderer_resize_callbacks;
	}

	std::span<vk2::Image> DeviceWindowVulkan::get_output_images()
	{
		return this->get_swapchain().get_images();
	}

	void DeviceWindowVulkan::on_resize(tz::Vec2ui dims)
	{
		TZ_PROFZONE("DeviceWindowVulkan OnResize", TZ_PROFCOL_YELLOW);
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
		vk2::Swapchain new_swapchain
		{{
			.device = &old_swapchain.get_device(),
			.swapchain_image_count_minimum = static_cast<std::uint32_t>(old_swapchain.get_image_views().size()),
			.image_format = old_swapchain.get_image_format(),
			.present_mode = old_swapchain.get_present_mode(),
			.old_swapchain = &old_swapchain
		}};
		std::swap(old_swapchain, new_swapchain);
		// Now notify all renderers.
		this->renderer_resize_callbacks
		({
			.new_dimensions = dims,
			.new_output_images = this->get_output_images()
		});
	}

	void DeviceWindowVulkan::register_resize()
	{
		this->on_resize_handle = tz::window().on_resize().add_callback([this](tz::Vec2ui dims){this->on_resize(dims);});
	}

	void DeviceWindowVulkan::unregister_resize()
	{
		if(this->is_resize_registered())
		{
			tz::window().on_resize().remove_callback(this->on_resize_handle);
			this->on_resize_handle = tz::nullhand;
		}
	}

	bool DeviceWindowVulkan::is_resize_registered() const
	{
		return this->on_resize_handle != tz::nullhand;
	}
	
	void DeviceWindowVulkan::reregister_resize()
	{
		this->unregister_resize();
		this->register_resize();
	}

//--------------------------------------------------------------------------------------------------
	DeviceRenderSchedulerVulkan::DeviceRenderSchedulerVulkan(const vk2::LogicalDevice& ldev, std::size_t frame_in_flight_count):
	image_available(),
	render_work_done(),
	frame_work()
	{
		for(std::size_t i = 0; i < frame_in_flight_count; i++)
		{
			this->image_available.emplace_back(ldev);
			this->render_work_done.emplace_back(ldev);
			this->frame_work.emplace_back(vk2::FenceInfo
			{
				.device = &ldev,
				.initially_signalled = true
			});
		}
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

	void DeviceRenderSchedulerVulkan::wait_frame_work_complete() const
	{
		for(const vk2::Fence& fence : this->frame_work)
		{
			fence.wait_until_signalled();
		}
	}

//--------------------------------------------------------------------------------------------------

	DeviceVulkan::DeviceVulkan():
	DeviceVulkan(vk2::get())
	{

	}

	DeviceVulkan::DeviceVulkan(const vk2::VulkanInstance& instance):
	device(DeviceVulkan::make_device(instance)),
	window_storage(this->device),
	scheduler(this->device, this->window_storage.get_output_images().size())
	{
		TZ_PROFZONE("Vulkan Frontend - DeviceVulkan Create", TZ_PROFCOL_YELLOW);
	}

	RendererVulkan DeviceVulkan::create_renderer(const RendererInfoVulkan& info)
	{
		TZ_PROFZONE("Vulkan Frontend - Renderer Create (via DeviceVulkan)", TZ_PROFCOL_YELLOW);
		return {info,
		{
			.device = &this->device,
			.output_images = this->window_storage.get_output_images(),
			.device_window = &this->window_storage,
			.device_scheduler = &this->scheduler,
			.resize_callback = &this->window_storage.resize_callback()
		}};
	}

	ImageFormat DeviceVulkan::get_window_format() const
	{
		return from_vk2(this->window_storage.get_format());
	}

	/*static*/vk2::LogicalDevice DeviceVulkan::make_device(const vk2::VulkanInstance& instance)
	{
		// First, create a LogicalDevice.
		// TODO: Don't just choose a device at random.
		vk2::PhysicalDeviceList pdevs = vk2::get_all_devices(instance);
		tz_assert(!pdevs.empty(), "Could not locate any physical devices at all. Your machine either needs a valid GPU, CPU or a virtualised device acting as the former. Please ensure your machine meets minimum system requirements.");
		vk2::PhysicalDevice pdev = *std::max_element(pdevs.begin(), pdevs.end(),
		[](const vk2::PhysicalDevice& a, const vk2::PhysicalDevice& b)
		{
			return rate_physical_device(a) < rate_physical_device(b);
		});
		tz_debug_report("Vulkan Device: Out of %zu device%s, chose \"%s\" because it had the highest rating (%u)", pdevs.length(), pdevs.length() == 1 ? "" : "s", pdev.get_info().name.c_str(), rate_physical_device(pdev));

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
				tz_debug_report("Developer: Warning: The selected device is not an Nvidia device. Non-nvidia devices are not tested at present, so if you do run into any issues please note on the bug report that your vendor is %s", vendor_name);
			}
		#endif

		vk2::DeviceExtensionList dev_exts = {};
		const vk2::DeviceFeatureField dev_feats =
		{
			vk2::DeviceFeature::BindlessDescriptors,
			vk2::DeviceFeature::ColourBlendLogicalOperations,
			vk2::DeviceFeature::NonSolidFillRasteriser,
			vk2::DeviceFeature::TessellationShaders,
			vk2::DeviceFeature::VertexPipelineResourceWrite,
			vk2::DeviceFeature::FragmentShaderResourceWrite
		};
		tz_assert(pdev.get_supported_features().contains(dev_feats), "One or both of DeviceFeatures 'BindlessDescriptors' and 'ColourBlendLogicalOperations' are not supported by this machine/driver. Please ensure your machine meets the system requirements.");
		dev_exts = {vk2::DeviceExtension::Swapchain};
		#if TZ_DEBUG
			dev_exts |= vk2::DeviceExtension::ShaderDebugPrint;
		#endif
		tz_assert(pdev.get_supported_extensions().contains(dev_exts), "One or more of the %zu required DeviceExtensions are not supported by this machine/driver. Please ensure your machine meets the system requirements.", dev_exts.count());
		return
		{{
			.physical_device = pdev,
			.extensions = dev_exts,
			.features = dev_feats
		}};
		
	}
}

#endif // TZ_VULKAN
