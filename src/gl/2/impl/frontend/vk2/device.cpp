#if TZ_VULKAN
#include "gl/2/impl/frontend/vk2/device.hpp"
#include "gl/2/impl/frontend/vk2/convert.hpp"
#include "gl/impl/backend/vk2/extensions.hpp"
#include "gl/impl/backend/vk2/hardware/physical_device.hpp"
#include "gl/impl/backend/vk2/image_format.hpp"
#include <variant>

namespace tz::gl2
{
	DeviceWindowVulkan::DeviceWindowVulkan():
	window_buf(std::monostate{})
	{}

	DeviceWindowVulkan::DeviceWindowVulkan(const vk2::LogicalDevice& device):
	DeviceWindowVulkan()
	{
		const vk2::VulkanInstance& instance = device.get_hardware().get_instance();
		if(instance.is_headless())
		{
			this->window_buf = vk2::Image
			{{
				.device = &device,
				.format = vk2::format_traits::get_mandatory_colour_attachment_formats().front(),
				/*TODO: Don't hardcode this jesus fucking christ*/
				.dimensions = {800u, 600u},
				.usage = {vk2::ImageUsage::ColourAttachment},
				.residency = vk2::MemoryResidency::GPU
			}};
		}
		else
		{
			// if we're not headless we must have a WindowSurface.
			tz_assert(instance.has_surface(), "DeviceWindowVulkan provided a VulkanInstance which is not headless, but doesn't have a WindowSurface attached. Please submit a bug report.");
			const vk2::WindowSurface& surface = instance.get_surface();

			// Create a swapchain.
			this->window_buf = vk2::Swapchain
			{{
				.device = &device,
				.swapchain_image_count_minimum = device.get_hardware().get_surface_capabilities().min_image_count,
				.image_format = device.get_hardware().get_supported_surface_formats().front(),
				.present_mode = device.get_hardware().get_supported_surface_present_modes().front()
			}};
		}
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

	DeviceVulkan::DeviceVulkan():
	DeviceVulkan(vk2::get())
	{

	}

	DeviceVulkan::DeviceVulkan(const vk2::VulkanInstance& instance):
	device(vk2::LogicalDevice::null()),
	window_storage()
	{
		// First, create a LogicalDevice.
		// TODO: Don't just choose a device at random.
		vk2::PhysicalDevice pdev = vk2::get_all_devices(instance).front();

		vk2::DeviceExtensionList dev_exts = {};
		const vk2::DeviceFeatureField dev_feats =
		{
			vk2::DeviceFeature::BindlessDescriptors,
			vk2::DeviceFeature::MultiDrawIndirect
		};
		tz_assert(pdev.get_supported_features().contains(dev_feats), "One or both of DeviceFeatures 'BindlessDescriptors' and 'MultiDrawIndirect' are not supported by this machine/driver. Please ensure your machine meets the system requirements.");
		if(!instance.is_headless())
		{
			dev_exts = {vk2::DeviceExtension::Swapchain};
		}
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

	RendererVulkan DeviceVulkan::create_renderer(RendererInfoVulkan& info)
	{
		std::span<vk2::Image> window_buffer_images;
		if(this->window_storage.as_swapchain() != nullptr)
		{
			window_buffer_images = this->window_storage.as_swapchain()->get_images();
		}
		else if(this->window_storage.as_image() != nullptr)
		{
			window_buffer_images = {this->window_storage.as_image(), 1};
		}
		
		return {info,
		{
			.device = &this->device,
			.output_images = window_buffer_images,
			.maybe_swapchain = this->window_storage.as_swapchain()
		}};
	}

	ImageFormat DeviceVulkan::get_window_format() const
	{
		return from_vk2(this->window_storage.get_format());
	}
}

#endif // TZ_VULKAN
