#include "tz/gl/impl/backend/vk2/framebuffer.hpp"
#include "tz/gl/impl/backend/vk2/swapchain.hpp"

void swapchain_image_framebuffer()
{
	// Creates a Framebuffer for a renderpass which draws into the first of the swapchain images.
	using namespace tz::gl::vk2;
	PhysicalDevice pdev = get_all_devices().front();

	LogicalDeviceInfo linfo;
	linfo.physical_device = pdev;
	linfo.extensions = {DeviceExtension::Swapchain};

	LogicalDevice ldev{linfo};

	SwapchainInfo sinfo;
	sinfo.device = &ldev;
	sinfo.swapchain_image_count_minimum = pdev.get_surface_capabilities().min_image_count;
	ImageFormat swapchain_image_format = pdev.get_supported_surface_formats().front();
	sinfo.image_format = swapchain_image_format;
	sinfo.present_mode = pdev.get_supported_surface_present_modes().front();

	Swapchain swapchain{sinfo};

	RenderPassBuilder pbuilder;
	pbuilder.set_device(ldev);
	pbuilder.with_attachment
	({
		.format = swapchain_image_format,
		.final_layout = ImageLayout::Present,
	})
	.with_subpass
	({
		.colour_attachments =
		{
			RenderPassInfo::AttachmentReference
			{
				.attachment_idx = 0,
				.current_layout = ImageLayout::ColourAttachment
			}
		}
	});

	RenderPass pass = pbuilder.build();
	
	FramebufferInfo fbinfo
	{
		.render_pass = &pass,
		.attachments = {&swapchain.get_image_views().front()},
		.dimensions = swapchain.get_dimensions()
	};

	Framebuffer swapchain_fb{fbinfo};
}

int main()
{
	tz::initialise
	({
		.name = "vk_framebuffer_test",
		.flags = {tz::ApplicationFlag::HiddenWindow}
	});
	{
		swapchain_image_framebuffer();
	}
	tz::terminate();
}
