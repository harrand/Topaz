#if TZ_VULKAN
#include "tz/gl/impl/vulkan/device2.hpp"
#include "tz/gl/impl/vulkan/detail/semaphore.hpp"
#include "tz/core/profile.hpp"
#include <algorithm>

namespace tz::gl
{
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

	void device_window::new_frame()
	{
		// this code is dubious - it might be completely wrong. feel free to delete if in doubt.
		tz::assert(!this->recent_acquire.has_value(), "New frame, but a swapchain image was acquired previously but never marked as used. Could be wrong, but potential logic error");
		this->recent_acquire = std::nullopt;
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
}

#endif // TZ_VULKAN
