#include "core/vector.hpp"
#include "gl/impl/backend/vk2/image.hpp"
#include "gl/impl/backend/vk2/image_format.hpp"
#include "gl/impl/backend/vk2/logical_device.hpp"

void basic_images()
{
	using namespace tz::gl::vk2;
	PhysicalDevice pdev = get_all_devices().front();
	LogicalDevice ldev
	{{
		.physical_device = pdev,
	}};

	ImageFormat fmt = format_traits::get_mandatory_colour_attachment_formats().front();
	
	Image basic_img
	{{
		.device = &ldev,
		.format = fmt,
		.dimensions = {2u, 2u},
		.usage = {ImageUsage::ColourAttachment},
		.residency = MemoryResidency::GPU,
		.image_tiling = ImageTiling::Optimal
	}};

	tz_assert(basic_img.get_dimensions() == tz::Vec2ui(2u, 2u), "Image had unexpected dimensions. Expected {%u, %u} but got {%u, %u}", 2u, 2u, basic_img.get_dimensions()[0], basic_img.get_dimensions()[1]);
	tz_assert(basic_img.get_layout() == ImageLayout::Undefined, "Image had unexpected initial layout. Expected ImageLayout::Undefined");
	tz_assert(basic_img.get_format() == fmt, "Image had unexpected format.");
}

int main()
{
	tz::GameInfo game{"vk_image_test", tz::Version{1, 0, 0}, tz::info()};
	tz::initialise(game, tz::ApplicationType::Headless);
	tz::gl::vk2::initialise(game, tz::ApplicationType::Headless);
	{
		basic_images();
	}
	tz::gl::vk2::terminate();
	tz::terminate();
}
