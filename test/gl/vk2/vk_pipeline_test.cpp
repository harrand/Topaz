#include "gl/impl/backend/vk2/pipeline_layout.hpp"

void basic_classic_pipeline_layout()
{
	using namespace tz::gl::vk2;

	PhysicalDevice pdev = get_all_devices().front();
	LogicalDeviceInfo linfo;
	linfo.physical_device = pdev;

	LogicalDevice ldev{linfo};
	{
		DescriptorLayoutBuilder builder;
		builder.set_device(ldev);
		// Firstly create a builder with no descriptors.	
		DescriptorLayoutInfo dinfo = builder.get_info();
		{
			DescriptorLayout dlayout{dinfo};

			PipelineLayoutInfo pinfo;
			pinfo.descriptor_layouts = {&dlayout};
			pinfo.logical_device = &ldev;

			PipelineLayout playout{pinfo};
		}
	}
}

int main()
{
	tz::initialise
	({
		.name = "vk_pipeline_test",
		.app_type = tz::ApplicationType::Headless
	});
	{
		basic_classic_pipeline_layout();
	}
	tz::terminate();
}
