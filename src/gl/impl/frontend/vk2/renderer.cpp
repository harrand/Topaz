#include "gl/impl/backend/vk2/image_view.hpp"
#if TZ_VULKAN
#include "gl/impl/frontend/vk2/renderer.hpp"

namespace tz::gl
{
	namespace detail
	{
		IOManager::IOManager(IOManagerInfo info):
		output_views(),
		render_pass(vk2::RenderPass::null())
		{
			tz_assert(!info.output_images.empty(), "RendererVulkan2 IOManager was not given any output images. Please submit a bug report.");
			for(vk2::Image& output_image : info.output_images)
			{
				this->output_views.push_back
				(vk2::ImageViewInfo{
					.image = &output_image,
					.aspect = vk2::ImageAspect::Colour
				});
			}

			tz_assert(std::equal(this->output_views.begin(), this->output_views.end(), this->output_views.begin(), [](const vk2::ImageView& a, const vk2::ImageView& b){return a.get_image().get_format() == b.get_image().get_format();}), "Detected that not every output image in a RendererVulkan2 has the same format. This is not submitted as RenderPasses would not be compatible. Please submit a bug report.");

			// Create a super basic RenderPass.
			{
				// We're matching the ImageFormat of the provided output image.
				vk2::ImageLayout final_layout;
				switch(info.output_type)
				{
					case RendererOutputType::Texture:
						final_layout = vk2::ImageLayout::ColourAttachment;
					break;
					case RendererOutputType::Window:
						final_layout = vk2::ImageLayout::Present;
					break;
					default:
						tz_error("Unknown RendererOutputType. Please submit a bug report.");
					break;
				}

				vk2::RenderPassBuilder rbuilder;
				rbuilder.set_device(*info.device);
				rbuilder.with_attachment
				({
					.format = this->output_views.front().get_image().get_format(),
					.initial_layout = vk2::ImageLayout::Undefined,
					.final_layout = final_layout
				});

				vk2::SubpassBuilder sbuilder;
				sbuilder.set_pipeline_context(vk2::PipelineContext::Graphics);
				sbuilder.with_colour_attachment
				({
					.attachment_idx = 0,
					.current_layout = vk2::ImageLayout::ColourAttachment
				});

				this->render_pass = rbuilder.with_subpass(sbuilder.build()).build();
			}
			// Now we can setup our Framebuffers. Each framebuffer will only attach to a single image, so if a Swapchain is involved under-the-hood then we are likely to have multiple of these. However in Headless contexts we expect only a single framebuffer.
			for(vk2::ImageView& output_view : this->output_views)
			{
				this->output_framebuffers.push_back
				(vk2::FramebufferInfo{
					.render_pass = &this->render_pass,
					.attachments = {&output_view},
					.dimensions = output_view.get_image().get_dimensions()
				});
			}
		}

		std::span<vk2::Framebuffer> IOManager::get_output_framebuffers()
		{
			return this->output_framebuffers;
		}
	}

	RendererVulkan2::RendererVulkan2(RendererBuilderVulkan2 builder, RendererBuilderDeviceInfoVulkan2 device_info):
	RendererBase(builder),
	io_manager
	({
		.device = device_info.device,
		.output_images = device_info.output_images,
		.output_type = builder.get_output()->get_type()
	})
	{

	}
}

#endif // TZ_VULKAN
