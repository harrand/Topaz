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
		DescriptorLayoutBuilderBindless build_all_bindless{ldev};
		// Firstly create a builder with no descriptors.	
		DescriptorLayoutInfo dinfo1 = builder.build();
		
		// Now one with a couple of resources.
		builder.with_descriptor(DescriptorType::Image).with_descriptor(DescriptorType::Sampler);
		DescriptorLayoutInfo dinfo2 = builder.build();
		// Then, one with just fucking everything.
		build_all
			.with_descriptor(DescriptorType::Sampler)
			.with_descriptor(DescriptorType::Image)
			.with_descriptor(DescriptorType::ImageWithSampler)
			.with_descriptor(DescriptorType::StorageImage)
			.with_descriptor(DescriptorType::UniformBuffer)
			.with_descriptor(DescriptorType::StorageBuffer);
		DescriptorLayoutInfo dinfo3 = build_all.build();
		// Finally, same as before but bindless (with arrays of small but >1 size).
		build_all_bindless
			.with_descriptor(DescriptorType::StorageBuffer, 255)
			.with_descriptor(DescriptorType::Image, 255)
			.with_descriptor(DescriptorType::StorageImage, 255)
			.with_descriptor(DescriptorType::Sampler, 255);
		DescriptorLayoutInfo dinfo4 = build_all_bindless.build();


		// Do some testing on the dinfos
		// All but dinfo4 should be Context::Classic
		tz_assert(dinfo1.context == DescriptorContext::Classic, "DescriptorLayoutInfo has wrong context type");
		tz_assert(dinfo2.context == DescriptorContext::Classic, "DescriptorLayoutInfo has wrong context type");
		tz_assert(dinfo3.context == DescriptorContext::Classic, "DescriptorLayoutInfo has wrong context type");
		tz_assert(dinfo4.context == DescriptorContext::Bindless, "DescriptorLayoutInfo has wrong context type");
		{
			DescriptorLayout dlayout1{dinfo1};
			DescriptorLayout dlayout2{dinfo2};
			DescriptorLayout dlayout3{dinfo3};
			DescriptorLayout dlayout4{dinfo4};
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
