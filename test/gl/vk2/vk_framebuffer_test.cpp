#include "gl/impl/backend/vk2/framebuffer.hpp"
#include "gl/impl/backend/vk2/swapchain.hpp"

void swapchain_image_framebuffer()
{
	// Creates a Framebuffer for a renderpass which draws into the first of the swapchain images.
	using namespace tz::gl::vk2;
	PhysicalDevice pdev = get_all_devices().front();

	LogicalDeviceInfo linfo;
	linfo.physical_device = pdev;
	linfo.surface = &get_window_surface();
	linfo.extensions = {DeviceExtension::Swapchain};

	LogicalDevice ldev{linfo};
	const WindowSurface& window_surf = tz::gl::vk2::get_window_surface();

	SwapchainInfo sinfo;
	sinfo.device = &ldev;
	sinfo.surface = &window_surf;
	sinfo.swapchain_image_count_minimum = pdev.get_surface_capabilities(window_surf).min_image_count;
	ImageFormat swapchain_image_format = pdev.get_supported_surface_formats(window_surf).front();
	sinfo.image_format = swapchain_image_format;
	sinfo.present_mode = pdev.get_supported_surface_present_modes(window_surf).front();

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
	tz::GameInfo game{"vk_framebuffer_test", tz::Version{1, 0, 0}, tz::info()};
	tz::initialise(game, tz::ApplicationType::HiddenWindowApplication);
	tz::gl::vk2::initialise(game, tz::ApplicationType::HiddenWindowApplication);
	{
		swapchain_image_framebuffer();
	}
	tz::gl::vk2::terminate();
	tz::terminate();
}
