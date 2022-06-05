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
	window_buf(std::monostate{})
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
		return !std::holds_alternative<std::monostate>(this->window_buf);
	}

	vk2::Swapchain* DeviceWindowVulkan::as_swapchain()
	{
		if(std::holds_alternative<vk2::Swapchain>(window_buf))
		{
			return &std::get<vk2::Swapchain>(window_buf);
		}
		return nullptr;
	}

	const vk2::Swapchain* DeviceWindowVulkan::as_swapchain() const
	{
		if(std::holds_alternative<vk2::Swapchain>(window_buf))
		{
			return &std::get<vk2::Swapchain>(window_buf);
		}
		return nullptr;
	}

	vk2::Image* DeviceWindowVulkan::as_image()
	{
		if(std::holds_alternative<vk2::Image>(window_buf))
		{
			return &std::get<vk2::Image>(window_buf);
		}
		return nullptr;
	}

	const vk2::Image* DeviceWindowVulkan::as_image() const
	{
		if(std::holds_alternative<vk2::Image>(window_buf))
		{
			return &std::get<vk2::Image>(window_buf);
		}
		return nullptr;
	}

	tz::Vec2ui DeviceWindowVulkan::get_dimensions() const
	{
		if(this->as_swapchain() != nullptr)
		{
			return this->as_swapchain()->get_dimensions();
		}
		else if(this->as_image() != nullptr)
		{
			return this->as_image()->get_dimensions();
		}
		return {0u, 0u};
	}

	vk2::ImageFormat DeviceWindowVulkan::get_format() const
	{
		if(this->as_swapchain() != nullptr)
		{
			return this->as_swapchain()->get_image_format();
		}
		else if(this->as_image() != nullptr)
		{
			return this->as_image()->get_format();
		}
		return vk2::ImageFormat::Undefined;
	}

	RendererResizeCallbackType& DeviceWindowVulkan::resize_callback()
	{
		return this->renderer_resize_callbacks;
	}

	std::span<vk2::Image> DeviceWindowVulkan::get_output_images()
	{
		if(this->as_swapchain() != nullptr)
		{
			return this->as_swapchain()->get_images();
		}
		else if(this->as_image() != nullptr)
		{
			return {this->as_image(), 1};
		}
		tz_error("Could not recognise window storage. Please submit a bug report.");
		return {};
	}

	void DeviceWindowVulkan::on_resize(tz::Vec2ui dims)
	{
		TZ_PROFZONE("DeviceWindowVulkan OnResize", TZ_PROFCOL_YELLOW);
		if(dims[0] == 0 || dims[0] == 0)
		{
			return;
		}
		// Assume we have a head, headless no support for resizeable output yet.
		tz_assert(this->as_swapchain() != nullptr, "Resizeable output for headless applications is not yet supported.");
		vk2::Swapchain& old_swapchain = *this->as_swapchain();
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

	DeviceVulkan::DeviceVulkan():
	DeviceVulkan(vk2::get())
	{

	}

	DeviceVulkan::DeviceVulkan(const vk2::VulkanInstance& instance):
	device(vk2::LogicalDevice::null()),
	window_storage()
	{
		TZ_PROFZONE("Vulkan Frontend - DeviceVulkan Create", TZ_PROFCOL_YELLOW);
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
			vk2::DeviceFeature::NonSolidFillRasteriser
		};
		tz_assert(pdev.get_supported_features().contains(dev_feats), "One or both of DeviceFeatures 'BindlessDescriptors' and 'ColourBlendLogicalOperations' are not supported by this machine/driver. Please ensure your machine meets the system requirements.");
		dev_exts = {vk2::DeviceExtension::Swapchain};
		#if TZ_DEBUG
			dev_exts |= vk2::DeviceExtension::ShaderDebugPrint;
		#endif
		tz_assert(pdev.get_supported_extensions().contains(dev_exts), "One or more of the %zu required DeviceExtensions are not supported by this machine/driver. Please ensure your machine meets the system requirements.", dev_exts.count());
		this->device =
		{{
			.physical_device = pdev,
			.extensions = dev_exts,
			.features = dev_feats
		}};
		
		// Now finally create the DeviceWindow.
		this->window_storage = {this->device};
	}

	RendererVulkan DeviceVulkan::create_renderer(const RendererInfoVulkan& info)
	{
		TZ_PROFZONE("Vulkan Frontend - Renderer Create (via DeviceVulkan)", TZ_PROFCOL_YELLOW);
		return {info,
		{
			.device = &this->device,
			.output_images = this->window_storage.get_output_images(),
			.maybe_swapchain = this->window_storage.as_swapchain(),
			.resize_callback = &this->window_storage.resize_callback()
		}};
	}

	ImageFormat DeviceVulkan::get_window_format() const
	{
		return from_vk2(this->window_storage.get_format());
	}

}

#endif // TZ_VULKAN
