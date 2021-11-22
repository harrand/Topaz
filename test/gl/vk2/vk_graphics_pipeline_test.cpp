#include "core/window.hpp"
#include "gl/impl/backend/vk2/graphics_pipeline.hpp"
#include "gl/impl/backend/vk2/image_format.hpp"

void basic_graphics_pipeline()
{
	using namespace tz::gl::vk2;
	PhysicalDevice pdev = get_all_devices().front();
	LogicalDeviceInfo linfo;
	linfo.physical_device = pdev;
	linfo.surface = &get_window_surface();

	LogicalDevice ldev{linfo};
	{
		VertexInputState vertex_input;
		InputAssembly input_assembly;
		ViewportState viewport = create_basic_viewport({tz::window().get_width(), tz::window().get_height()});
		RasteriserState rasteriser;
		MultisampleState multisample;
		DepthStencilState depth_stencil;
		ColourBlendState colour_blend;
		DynamicState dynamic;

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
			.vertex_input_state = &vertex_input,
			.input_assembly = &input_assembly,
			.viewport_state = &viewport,
			.rasteriser_state = &rasteriser,
			.multisample_state = &multisample,
			.depth_stencil_state = &depth_stencil,
			.colour_blend_state = &colour_blend,
			.dynamic_state = &dynamic,

			.pipeline_layout = &pipeline_layout,
			.render_pass = &pass,

			.device = &ldev
		};
	}
}

int main()
{
	tz::GameInfo game{"vk_descriptor_test", tz::Version{1, 0, 0}, tz::info()};
	tz::initialise(game, tz::ApplicationType::HiddenWindowApplication);
	tz::gl::vk2::initialise(game, tz::ApplicationType::HiddenWindowApplication);
	{
		basic_graphics_pipeline();
	}
	tz::gl::vk2::terminate();
	tz::terminate();
}
