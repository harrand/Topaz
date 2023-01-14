#if TZ_VULKAN
#include "hdk/profile.hpp"
#include "tz/gl/impl/vulkan/detail/image.hpp"
#include "tz/gl/impl/vulkan/detail/swapchain.hpp"
#include "tz/gl/impl/vulkan/detail/semaphore.hpp"
#include "tz/gl/impl/vulkan/detail/fence.hpp"

namespace tz::gl::vk2
{	
	Swapchain::Swapchain(SwapchainInfo info):
	swapchain(VK_NULL_HANDLE),
	info(info),
	dimensions(),
	swapchain_images(),
	swapchain_image_views()
	{
		HDK_PROFZONE("Vulkan Backend - Swapchain Create", 0xFFAA0000);
		hdk::assert(this->info.device != nullptr, "SwapchainInfo contained null LogicalDevice. Please submi a bug report.");
		const LogicalDevice& ldev = *this->info.device;
		const PhysicalDevice& pdev = ldev.get_hardware();

		// Ensure the swapchain info specifies meets hardware requirements:
		// Check the Swapchain extension is enabled.
		hdk::assert(ldev.get_extensions().contains(DeviceExtension::Swapchain), "Attempted to create Swapchain, but owning LogicalDevice does not have DeviceExtension::Swapchain enabled!");

		// Ensure the image count is reasonable (not too low or high)
		PhysicalDeviceSurfaceCapabilityInfo pdev_surface_cap = pdev.get_surface_capabilities();
		hdk::assert(pdev_surface_cap.min_image_count <= info.swapchain_image_count_minimum, "Swapchain image count too low (hardware minimum = %u, requested %u). Please submit a bug report.", pdev_surface_cap.min_image_count, info.swapchain_image_count_minimum);
		hdk::assert(info.swapchain_image_count_minimum <= pdev_surface_cap.max_image_count, "Swapchain image count too high (hardware maximum = %u, requested %u). Please submit a bug report.", pdev_surface_cap.max_image_count, info.swapchain_image_count_minimum);
		// Now ensure the PhysicalDevice supports the provided ImageFormat
		hdk::assert(pdev.get_supported_surface_formats().contains(this->info.image_format), "Swapchain provided ImageFormat which the PhysicalDevice does not support. Please submit a bug report.");
		const WindowSurface& surface = pdev.get_instance().get_surface();

		// Once we're ready, find out the surface extent we want.
		// The PhysicalDevice might have a number for us, but it may not. If not we just use the window dimensions.
		VkExtent2D extent;
		{
			if(pdev_surface_cap.maybe_surface_dimensions.has_value())
			{
				hdk::vector<std::uint32_t, 2> current_extent = pdev_surface_cap.maybe_surface_dimensions.value().current_extent;
				extent.width = current_extent[0];
				extent.height = current_extent[1];
			}
			else
			{
				extent.width = static_cast<std::uint32_t>(surface.get_window().get_width());
				extent.height = static_cast<std::uint32_t>(surface.get_window().get_height());
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
		if(info.old_swapchain != nullptr)
		{
			create.oldSwapchain = info.old_swapchain->native();
		}

		VkResult res = vkCreateSwapchainKHR(ldev.native(), &create, nullptr, &this->swapchain);
		switch(res)
		{
			case VK_SUCCESS:
				// don't do anything.	
			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				hdk::error("Ran out of host memory (RAM) while trying to create Swapchain. Ensure your device meets minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				hdk::error("Ran out of device memory (VRAM) while trying to create Swapchain. Ensure your device meets minimum requirements");
			break;
			case VK_ERROR_DEVICE_LOST:
				hdk::error("Device lost whilst trying to create a Swapchain. Possible hardware fault. Please be aware: Device loss is serious and further attempts to run the engine may cause serious hazards, such as operating system crash. Submit a bug report but do not attempt to reproduce the issue.");
			break;
			case VK_ERROR_SURFACE_LOST_KHR:
				hdk::error("Surface lost whilst trying to create a Swapchain. Please submit a bug report (Please explain in detail what you were doing, this is window-related)");
			break;
			case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
				hdk::error("Tried to create a Swapchain. Requested window is already in-use. If you're somehow using more than 1 window while running this application, this is probably why - Try reproducing the issue in a single window. If you're not doing anything special, please submit a bug report");
			break;
			case VK_ERROR_INITIALIZATION_FAILED:
				hdk::error("Tried to create a Swapchain, but something went wrong for implementation-specified reasons. Unfortunately this error code is extremely cryptic. Please submit a bug report (although please don't get your hopes up)");
			break;
			default:
				hdk::error("Failed to create a Swapchain, and the error code is unrecognised. Either we are missing a return code case, or something has gone very, very wrong. Please submit a bug report.");
			break;
		}

		this->dimensions[0] = static_cast<unsigned int>(extent.width);
		this->dimensions[1] = static_cast<unsigned int>(extent.height);
		this->initialise_images();
	}

	Swapchain::Swapchain(Swapchain&& move):
	swapchain(VK_NULL_HANDLE),
	info(),
	dimensions(),
	swapchain_images(),
	swapchain_image_views()
	{
		*this = std::move(move);
	}

	Swapchain::~Swapchain()
	{
		if(this->swapchain != VK_NULL_HANDLE)
		{
			hdk::assert(this->info.device != nullptr, "Cannot destroy Swapchain because we are unaware of the spawning LogicalDevice. Please submit a bug report");
			vkDestroySwapchainKHR(this->info.device->native(), this->swapchain, nullptr);
			this->swapchain = VK_NULL_HANDLE;
		}
	}

	Swapchain& Swapchain::operator=(Swapchain&& rhs)
	{
		std::swap(this->swapchain, rhs.swapchain);
		std::swap(this->info, rhs.info);
		std::swap(this->dimensions, rhs.dimensions);
		std::swap(this->swapchain_images, rhs.swapchain_images);
		std::swap(this->swapchain_image_views, rhs.swapchain_image_views);
		return *this;
	}

	const LogicalDevice& Swapchain::get_device() const
	{
		hdk::assert(!this->is_null(), "Tried to retrieve LogicalDevice from a null Swapchain. Please submit a bug report.");
		const LogicalDevice* ldev = this->info.device;
		hdk::assert(ldev != nullptr && !ldev->is_null(), "SwapchainInfo contained nullptr or null LogicalDevice");
		return *ldev;
	}

	Swapchain::ImageAcquisitionResult Swapchain::acquire_image(const Swapchain::ImageAcquisition& acquire)
	{
		HDK_PROFZONE("Vulkan Backend - Swapchain Image Acquire", 0xFFAA0000);
		BinarySemaphore::NativeType signal_semaphore_native = VK_NULL_HANDLE;
		Fence::NativeType signal_fence_native = VK_NULL_HANDLE;

		if(acquire.signal_semaphore != nullptr)
		{
			signal_semaphore_native = acquire.signal_semaphore->native();
		}
		if(acquire.signal_fence != nullptr)
		{
			signal_fence_native = acquire.signal_fence->native();
		}

		Swapchain::ImageAcquisitionResult result;
		vkAcquireNextImageKHR(this->get_device().native(), this->swapchain, acquire.timeout, signal_semaphore_native, signal_fence_native, &result.image_index);
		return result;
	}

	Swapchain Swapchain::null()
	{
		return {};
	}

	bool Swapchain::is_null() const
	{
		return this->swapchain == VK_NULL_HANDLE;
	}

	Swapchain::NativeType Swapchain::native() const
	{
		return this->swapchain;
	}

	std::span<const Image> Swapchain::get_images() const
	{
		return this->swapchain_images;
	}

	std::span<Image> Swapchain::get_images() 
	{
		return this->swapchain_images;
	}

	std::span<const ImageView> Swapchain::get_image_views() const
	{
		return this->swapchain_image_views;
	}

	std::span<ImageView> Swapchain::get_image_views()
	{
		return this->swapchain_image_views;
	}

	ImageFormat Swapchain::get_image_format() const
	{
		return this->info.image_format;
	}

	SurfacePresentMode Swapchain::get_present_mode() const
	{
		return this->info.present_mode;
	}

	hdk::vec2ui Swapchain::get_dimensions() const
	{
		return this->dimensions;
	}

	Swapchain::Swapchain():
	swapchain(VK_NULL_HANDLE),
	info(){}

	void Swapchain::initialise_images()
	{
		hdk::assert(!this->is_null(), "Tried to initialise Swapchain images, but the Swapchain is null. Please submit a bug report.");
		if(!this->swapchain_images.empty())
		{
			this->swapchain_images.clear();
			this->swapchain_image_views.clear();
		}

		std::uint32_t true_image_count;
		vkGetSwapchainImagesKHR(this->get_device().native(), this->swapchain, &true_image_count, nullptr);
		this->swapchain_images.reserve(true_image_count);
		for(std::uint32_t i = 0; std::cmp_less(i, true_image_count); i++)
		{
			SwapchainImageInfo sinfo;
			sinfo.swapchain = this;
			sinfo.image_index = i;

			Image& img = this->swapchain_images.emplace_back(sinfo);

			ImageViewInfo view_info;
			view_info.image = &img;
			view_info.aspect = ImageAspectFlag::Colour;
			this->swapchain_image_views.emplace_back(view_info);
		}
	}
}

#endif // TZ_VULKAN
