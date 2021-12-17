#include "core/window.hpp"
#include "gl/impl/backend/vk2/graphics_pipeline.hpp"
#include "gl/impl/backend/vk2/image_format.hpp"

void basic_graphics_pipeline()
{
	using namespace tz::gl::vk2;
	PhysicalDevice pdev = get_all_devices().front();
	LogicalDeviceInfo linfo;
	linfo.physical_device = pdev;

	LogicalDevice ldev{linfo};
	{
		DescriptorLayoutBuilder lbuilder;
		lbuilder.set_device(ldev);
		std::vector<DescriptorLayout> dlayouts;
		dlayouts.push_back(lbuilder.build());

		PipelineLayoutInfo plinfo
		{
			.descriptor_layouts = dlayouts,
			.logical_device = &ldev
		};
		PipelineLayout pipeline_layout{plinfo};

		RenderPassBuilder pbuilder;
		pbuilder.set_device(ldev);
		pbuilder.with_attachment
		({
			.format = format_traits::get_mandatory_colour_attachment_formats().front(),
			.initial_layout = ImageLayout::Undefined,
			.final_layout = ImageLayout::ColourAttachment
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

		GraphicsPipelineInfo ginfo
		{
			.state = PipelineState
			{
				.viewport = create_basic_viewport({1.0f, 1.0f})
			},
			.pipeline_layout = &pipeline_layout,
			.render_pass = &pass,
			.device = &ldev
		};
	}
}

int main()
{
	tz::GameInfo game{"vk_descriptor_test", tz::Version{1, 0, 0}, tz::info()};
	tz::initialise(game, tz::ApplicationType::Headless);
	tz::gl::vk2::initialise(game, tz::ApplicationType::Headless);
	{
		basic_graphics_pipeline();
	}
	tz::gl::vk2::terminate();
	tz::terminate();
}
