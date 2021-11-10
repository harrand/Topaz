#include "core/tz.hpp"
#include "gl/impl/backend/vk2/descriptors.hpp"

void basic_descriptor_layout()
{
	using namespace tz::gl::vk2;
	PhysicalDevice pdev = get_all_devices().front();
	LogicalDeviceInfo linfo;
	linfo.physical_device = pdev;
	linfo.surface = &get_window_surface();

	LogicalDevice ldev{linfo};
	{
		DescriptorLayoutBuilder builder{ldev};
		DescriptorLayoutBuilder build_all{ldev};
		// Firstly create a builder with no descriptors.	
		DescriptorLayoutInfo dinfo1 = builder.build();
		
		// Now one with a couple of resources.
		builder.with_descriptor(DescriptorType::Image).with_descriptor(DescriptorType::Sampler);
		DescriptorLayoutInfo dinfo2 = builder.build();
		// Finally, one with just fucking everything.
		build_all
			.with_descriptor(DescriptorType::Sampler)
			.with_descriptor(DescriptorType::Image)
			.with_descriptor(DescriptorType::ImageWithSampler)
			.with_descriptor(DescriptorType::StorageImage)
			.with_descriptor(DescriptorType::UniformBuffer)
			.with_descriptor(DescriptorType::StorageBuffer);
		DescriptorLayoutInfo dinfo3 = build_all.build();
		{
			DescriptorLayout dlayout1{dinfo1};
			DescriptorLayout dlayout2{dinfo2};
			DescriptorLayout dlayout3{dinfo3};
		}
	}
}

int main()
{
	tz::GameInfo game{"vk_descriptor_test", tz::Version{1, 0, 0}, tz::info()};
	tz::initialise(game, tz::ApplicationType::HiddenWindowApplication);
	tz::gl::vk2::initialise(game, tz::ApplicationType::HiddenWindowApplication);
	{
		basic_descriptor_layout();
	}
	tz::gl::vk2::terminate();
	tz::terminate();
}
