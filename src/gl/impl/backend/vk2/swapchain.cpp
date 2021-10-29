#if TZ_VULKAN
#include "gl/impl/backend/vk2/swapchain.hpp"

namespace tz::gl::vk2
{	
	Swapchain::Swapchain(SwapchainInfo info):
	swapchain(VK_NULL_HANDLE),
	info(info)
	{
		tz_assert(this->info.device != nullptr, "SwapchainInfo contained null LogicalDevice. Please submi a bug report.");
		tz_assert(this->info.surface != nullptr, "SwapchainInfo contained null WindowSurface. Please submit a bug report.");
		const LogicalDevice& ldev = *this->info.device;
		const PhysicalDevice& pdev = ldev.get_hardware();
		const WindowSurface& surface = *this->info.surface;

		// Ensure the swapchain info specifies meets hardware requirements:
		// Check the Swapchain extension is enabled.
		tz_assert(ldev.get_extensions().contains(DeviceExtension::Swapchain), "Attempted to create Swapchain, but owning LogicalDevice does not have DeviceExtension::Swapchain enabled!");

		// Ensure the image count is reasonable (not too low or high)
		PhysicalDeviceSurfaceCapabilityInfo pdev_surface_cap = pdev.get_surface_capabilities(surface);
		tz_assert(pdev_surface_cap.min_image_count <= info.swapchain_image_count_minimum, "Swapchain image count too low (hardware minimum = %u, requested %u). Please submit a bug report.", pdev_surface_cap.min_image_count, info.swapchain_image_count_minimum);
		tz_assert(info.swapchain_image_count_minimum <= pdev_surface_cap.max_image_count, "Swapchain image count too high (hardware maximum = %u, requested %u). Please submit a bug report.", pdev_surface_cap.max_image_count, info.swapchain_image_count_minimum);
		// Now ensure the PhysicalDevice supports the provided ImageFormat
		tz_assert(pdev.get_supported_surface_formats(surface).contains(this->info.image_format), "Swapchain provided ImageFormat which the PhysicalDevice does not support. Please submit a bug report.");

		// Once we're ready, find out the surface extent we want.
		// The PhysicalDevice might have a number for us, but it may not. If not we just use the window dimensions.
		VkExtent2D extent;
		{
			if(pdev_surface_cap.maybe_surface_dimensions.has_value())
			{
				tz::Vector<std::uint32_t, 2> current_extent = pdev_surface_cap.maybe_surface_dimensions.value().current_extent;
				extent.width = current_extent[0];
				extent.height = current_extent[1];
			}
			else
			{
				extent.width = static_cast<std::uint32_t>(tz::window().get_width());
				extent.height = static_cast<std::uint32_t>(tz::window().get_height());
			}
		}

		VkSwapchainCreateInfoKHR create{};
		create.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create.surface = surface.native();
		create.minImageCount = this->info.swapchain_image_count_minimum;
		create.imageFormat = static_cast<VkFormat>(this->info.image_format);
		create.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR; 
		create.imageExtent = extent;
		create.imageArrayLayers = 1;
		create.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		create.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create.queueFamilyIndexCount = 0;
		create.pQueueFamilyIndices = nullptr;
		create.preTransform = pdev_surface_cap.current_transform;
		create.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		create.presentMode = static_cast<VkPresentModeKHR>(this->info.present_mode);
		create.clipped = VK_TRUE;
		create.oldSwapchain = VK_NULL_HANDLE;

		VkResult res = vkCreateSwapchainKHR(ldev.native(), &create, nullptr, &this->swapchain);
		switch(res)
		{
			case VK_SUCCESS:
				// don't do anything.	
			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				tz_error("Ran out of host memory (RAM) while trying to create Swapchain. Ensure your device meets minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				tz_error("Ran out of device memory (VRAM) while trying to create Swapchain. Ensure your device meets minimum requirements");
			break;
			case VK_ERROR_DEVICE_LOST:
				tz_error("Device lost whilst trying to create a Swapchain. Possible hardware fault. Please be aware: Device loss is serious and further attempts to run the engine may cause serious hazards, such as operating system crash. Submit a bug report but do not attempt to reproduce the issue.");
			break;
			case VK_ERROR_SURFACE_LOST_KHR:
				tz_error("Surface lost whilst trying to create a Swapchain. Please submit a bug report (Please explain in detail what you were doing, this is window-related)");
			break;
			case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
				tz_error("Tried to create a Swapchain. Requested window is already in-use. If you're somehow using more than 1 window while running this application, this is probably why - Try reproducing the issue in a single window. If you're not doing anything special, please submit a bug report");
			break;
			case VK_ERROR_INITIALIZATION_FAILED:
				tz_error("Tried to create a Swapchain, but something went wrong for implementation-specified reasons. Unfortunately this error code is extremely cryptic. Please submit a bug report (although please don't get your hopes up)");
			break;
			default:
				tz_error("Failed to create a Swapchain, and the error code is unrecognised. Either we are missing a return code case, or something has gone very, very wrong. Please submit a bug report.");
			break;
		}
	}

	Swapchain::Swapchain(Swapchain&& move):
	swapchain(VK_NULL_HANDLE),
	info()
	{
		*this = std::move(move);
	}

	Swapchain::~Swapchain()
	{
		if(this->swapchain != VK_NULL_HANDLE)
		{
			tz_assert(this->info.device != nullptr, "Cannot destroy Swapchain because we are unaware of the spawning LogicalDevice. Please submit a bug report");
			vkDestroySwapchainKHR(this->info.device->native(), this->swapchain, nullptr);
			this->swapchain = VK_NULL_HANDLE;
		}
	}

	Swapchain& Swapchain::operator=(Swapchain&& rhs)
	{
		std::swap(this->swapchain, rhs.swapchain);
		std::swap(this->info, rhs.info);
		return *this;
	}

	Swapchain Swapchain::null()
	{
		return {};
	}

	bool Swapchain::is_null() const
	{
		return this->swapchain == VK_NULL_HANDLE;
	}

	VkSwapchainKHR Swapchain::native() const
	{
		return this->swapchain;
	}

	Swapchain::Swapchain():
	swapchain(VK_NULL_HANDLE),
	info(){}
}

#endif // TZ_VULKAN
