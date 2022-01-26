#include "gl/impl/backend/vk2/render_pass.hpp"

void basic_render_pass()
{
	using namespace tz::gl::vk2;
	PhysicalDevice pdev = get_all_devices().front();
	LogicalDeviceInfo linfo;
	linfo.physical_device = pdev;

	LogicalDevice ldev{linfo};
	{
		RenderPassBuilder builder;

		SubpassBuilder sbuilder;
		sbuilder.set_pipeline_context(PipelineContext::Graphics);
		sbuilder.with_colour_attachment
		({
			.attachment_idx = 0,
			.current_layout = ImageLayout::General
		});

		builder.with_attachment
		({
			.format = ImageFormat::R8,
			.final_layout = ImageLayout::General
		});
		builder.with_subpass(sbuilder.build());
		builder.set_device(ldev);
		RenderPass pass = builder.build();
	}
}

void complicated_render_pass()
{
	using namespace tz::gl::vk2;
	PhysicalDevice pdev = get_all_devices().front();
	LogicalDeviceInfo linfo;
	linfo.physical_device = pdev;

	LogicalDevice ldev{linfo};
	{
		RenderPassBuilder builder;

		SubpassBuilder sbuilder;
		sbuilder.set_pipeline_context(PipelineContext::Graphics);
		sbuilder.with_colour_attachment
		({
			.attachment_idx = 0,
			.current_layout = ImageLayout::General
		})
		.with_colour_attachment
		({
			.attachment_idx = 1,
			.current_layout = ImageLayout::ColourAttachment
		});

		builder.with_attachment
		({
			.format = ImageFormat::R8,
			.final_layout = ImageLayout::General
		})
		.with_attachment
		({
			.format = ImageFormat::RGBA32_sRGB,
			.final_layout = ImageLayout::ColourAttachment
		});
		builder.with_subpass(sbuilder.build());
		builder.set_device(ldev);
		RenderPass pass = builder.build();
	}
}

int main()
{
	tz::GameInfo game{"vk_render_pass_test", tz::Version{1, 0, 0}, tz::info()};
	tz::initialise(game, tz::ApplicationType::Headless);
	{
		basic_render_pass();
		complicated_render_pass();
	}
	tz::terminate();
}
