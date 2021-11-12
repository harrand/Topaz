#include "core/tz.hpp"
#include "gl/impl/backend/vk2/descriptors.hpp"

void basic_classic_descriptor_layout()
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
		// Then, one with just fucking everything.
		build_all
			.with_descriptor(DescriptorType::Sampler)
			.with_descriptor(DescriptorType::Image)
			.with_descriptor(DescriptorType::ImageWithSampler)
			.with_descriptor(DescriptorType::StorageImage)
			.with_descriptor(DescriptorType::UniformBuffer)
			.with_descriptor(DescriptorType::StorageBuffer);
		DescriptorLayoutInfo dinfo3 = build_all.build();

		// Do some testing on the dinfos
		tz_assert(dinfo1.context == DescriptorContext::Classic, "DescriptorLayoutInfo has wrong context type");
		tz_assert(dinfo2.context == DescriptorContext::Classic, "DescriptorLayoutInfo has wrong context type");
		tz_assert(dinfo3.context == DescriptorContext::Classic, "DescriptorLayoutInfo has wrong context type");
		{
			DescriptorLayout dlayout1{dinfo1};
			DescriptorLayout dlayout2{dinfo2};
			DescriptorLayout dlayout3{dinfo3};
		}
	}
}

void basic_bindless_descriptor_layout()
{
	using namespace tz::gl::vk2;
	PhysicalDevice pdev = get_all_devices().front();
	if(!pdev.get_supported_features().contains(DeviceFeature::BindlessDescriptors))
	{
		// Can't test bindless if the PhysicalDevice doesn't support it
		return;
	}
	LogicalDeviceInfo linfo;
	linfo.physical_device = pdev;
	linfo.surface = &get_window_surface();
	linfo.features = {DeviceFeature::BindlessDescriptors};

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

void classic_pool_creation()
{
	using namespace tz::gl::vk2;
	PhysicalDevice pdev = get_all_devices().front();
	LogicalDeviceInfo linfo;
	linfo.physical_device = pdev;
	linfo.surface = &get_window_surface();

	LogicalDevice ldev{linfo};
	{
		DescriptorLayoutBuilder builder{ldev};
		builder.with_descriptor(DescriptorType::UniformBuffer);
		DescriptorLayoutInfo dinfo = builder.build();

		DescriptorPoolInfo pinfo = tz::gl::vk2::create_pool_for_layout(dinfo, 1);
		DescriptorPool pool{pinfo};
	}
}

void bindless_pool_creation()
{
	using namespace tz::gl::vk2;
	PhysicalDevice pdev = get_all_devices().front();
	if(!pdev.get_supported_features().contains(DeviceFeature::BindlessDescriptors))
	{
		return;
	}
	LogicalDeviceInfo linfo;
	linfo.physical_device = pdev;
	linfo.surface = &get_window_surface();
	linfo.features = {DeviceFeature::BindlessDescriptors};

	LogicalDevice ldev{linfo};
	{
		DescriptorLayoutBuilderBindless builder{ldev};
		builder.with_descriptor(DescriptorType::UniformBuffer, 10);
		DescriptorLayoutInfo dinfo = builder.build();

		DescriptorPoolInfo pinfo = tz::gl::vk2::create_pool_for_layout(dinfo, 1);
		DescriptorPool pool{pinfo};
	}
}

void classic_pool_usage_one_set()
{
	// Create pool large enough for one UniformBuffer. Then create a DescriptorSet for it.
	using namespace tz::gl::vk2;
	PhysicalDevice pdev = get_all_devices().front();
	LogicalDeviceInfo linfo;
	linfo.physical_device = pdev;
	linfo.surface = &get_window_surface();

	LogicalDevice ldev{linfo};
	{
		DescriptorLayoutBuilder builder{ldev};
		builder.with_descriptor(DescriptorType::UniformBuffer);
		DescriptorLayoutInfo dinfo = builder.build();

		DescriptorPoolInfo pinfo = tz::gl::vk2::create_pool_for_layout(dinfo, 1);
		DescriptorPool pool{pinfo};

		DescriptorLayout dlayout{dinfo};

		DescriptorPool::AllocateInfo alloc;
		alloc.set_layouts = {&dlayout};
		tz::BasicList<DescriptorSet> result_sets = pool.allocate_sets(alloc);
		tz_assert(result_sets.length() == 1, "DescriptorPool allocation returned unexpected number of DescriptorSets. Expected %zu, but got %zu", 1, result_sets.length());
	}
}

void classic_pool_usage_full_sets()
{
	// Create pool large enough 6x {1 UniformBuffer, 1 CombinedImageSampler}. Then create 6 DescriptorSet for it.
	constexpr int set_count = 6;
	using namespace tz::gl::vk2;
	PhysicalDevice pdev = get_all_devices().front();
	LogicalDeviceInfo linfo;
	linfo.physical_device = pdev;
	linfo.surface = &get_window_surface();

	LogicalDevice ldev{linfo};
	{
		DescriptorLayoutBuilder builder{ldev};
		builder.with_descriptor(DescriptorType::UniformBuffer);
		builder.with_descriptor(DescriptorType::ImageWithSampler);
		DescriptorLayoutInfo dinfo = builder.build();

		DescriptorPoolInfo pinfo = tz::gl::vk2::create_pool_for_layout(dinfo, set_count);
		DescriptorPool pool{pinfo};

		DescriptorLayout dlayout{dinfo};

		DescriptorPool::AllocateInfo alloc;
		for(std::size_t i = 0; i < set_count; i++)
		{
			alloc.set_layouts.add(&dlayout);
		}
		tz::BasicList<DescriptorSet> result_sets = pool.allocate_sets(alloc);
		tz_assert(result_sets.length() == set_count, "DescriptorPool allocation returned unexpected number of DescriptorSets. Expected %d, but got %zu", set_count, result_sets.length());
	}
}

void bindless_pool_usage_one_set()
{
	// Create pool large enough for one StorageBuffer. Then create a DescriptorSet for it.
	using namespace tz::gl::vk2;
	PhysicalDevice pdev = get_all_devices().front();
	if(!pdev.get_supported_features().contains(DeviceFeature::BindlessDescriptors))
	{
		return;
	}
	LogicalDeviceInfo linfo;
	linfo.physical_device = pdev;
	linfo.surface = &get_window_surface();
	linfo.features = {DeviceFeature::BindlessDescriptors};

	LogicalDevice ldev{linfo};
	{
		DescriptorLayoutBuilderBindless builder{ldev};
		builder.with_descriptor(DescriptorType::StorageBuffer, 1);
		DescriptorLayoutInfo dinfo = builder.build();

		DescriptorPoolInfo pinfo = tz::gl::vk2::create_pool_for_layout(dinfo, 1);
		DescriptorPool pool{pinfo};

		DescriptorLayout dlayout{dinfo};

		DescriptorPool::AllocateInfo alloc;
		alloc.set_layouts = {&dlayout};
		tz::BasicList<DescriptorSet> result_sets = pool.allocate_sets(alloc);
		tz_assert(result_sets.length() == 1, "DescriptorPool allocation returned unexpected number of DescriptorSets. Expected %zu, but got %zu", 1, result_sets.length());
	}
}

void bindless_pool_usage_full_sets()
{
	// Create pool large enough 6x {49 StorageBuffer, 3 CombinedImageSampler}. Then create 6 DescriptorSet for it.
	constexpr int set_count = 6;
	constexpr int buf_count = 49;
	constexpr int image_count = 3;

	using namespace tz::gl::vk2;
	PhysicalDevice pdev = get_all_devices().front();
	if(!pdev.get_supported_features().contains(DeviceFeature::BindlessDescriptors))
	{
		return;
	}
	LogicalDeviceInfo linfo;
	linfo.physical_device = pdev;
	linfo.surface = &get_window_surface();
	linfo.features = {DeviceFeature::BindlessDescriptors};

	LogicalDevice ldev{linfo};
	{
		DescriptorLayoutBuilderBindless builder{ldev};
		builder.with_descriptor(DescriptorType::StorageBuffer, buf_count);
		builder.with_descriptor(DescriptorType::ImageWithSampler, image_count);
		DescriptorLayoutInfo dinfo = builder.build();

		DescriptorPoolInfo pinfo = tz::gl::vk2::create_pool_for_layout(dinfo, set_count);
		DescriptorPool pool{pinfo};

		DescriptorLayout dlayout{dinfo};

		DescriptorPool::AllocateInfo alloc;
		for(std::size_t i = 0; i < set_count; i++)
		{
			alloc.set_layouts.add(&dlayout);
		}
		tz::BasicList<DescriptorSet> result_sets = pool.allocate_sets(alloc);
		tz_assert(result_sets.length() == set_count, "DescriptorPool allocation returned unexpected number of DescriptorSets. Expected %d, but got %zu", set_count, result_sets.length());
	}
}

void semantics()
{
	using namespace tz::gl::vk2;	
	// Ensure constexpr stuff
	static_assert(!tz::copyable<DescriptorLayout>, "DescriptorLayout is wrongly copyable");
	static_assert(tz::moveable<DescriptorLayout>, "DescriptorLayout is wrongly not moveable");

	static_assert(!tz::copyable<DescriptorPool>, "DescriptorPool is wrongly copyable");
	static_assert(tz::moveable<DescriptorPool>, "DescriptorPool is wrongly not moveable");

	PhysicalDevice pdev = get_all_devices().front();
	if(!pdev.get_supported_features().contains(DeviceFeature::BindlessDescriptors))
	{
		return;
	}
	LogicalDeviceInfo linfo;
	linfo.physical_device = pdev;
	linfo.surface = &get_window_surface();
	linfo.features = {DeviceFeature::BindlessDescriptors};

	LogicalDevice ldev{linfo};
	// Semantics for DescriptorLayout
	{
		DescriptorLayoutBuilder builder{ldev};
		builder.with_descriptor(DescriptorType::UniformBuffer);
		DescriptorLayoutInfo dinfo = builder.build();

		DescriptorLayout d1{dinfo};
		DescriptorLayout d2 = DescriptorLayout::null();
		tz_assert(!d1.is_null(), "DescriptorLayout wrongly considered null");
		tz_assert(d2.is_null(), "DescriptorLayout wrongly considered not-null");
		d2 = std::move(d1);
		tz_assert(!d2.is_null(), "DescriptorLayout wrongly considered null after move assign");
		DescriptorLayout d3{std::move(d2)};
		tz_assert(!d3.is_null(), "DescriptorLayout wrongly considered null after move construction");
	}
}

int main()
{
	tz::GameInfo game{"vk_descriptor_test", tz::Version{1, 0, 0}, tz::info()};
	tz::initialise(game, tz::ApplicationType::HiddenWindowApplication);
	tz::gl::vk2::initialise(game, tz::ApplicationType::HiddenWindowApplication);
	{
		basic_classic_descriptor_layout();
		basic_bindless_descriptor_layout();
		classic_pool_creation();
		classic_pool_usage_one_set();
		classic_pool_usage_full_sets();
		bindless_pool_creation();
		bindless_pool_usage_one_set();
		bindless_pool_usage_full_sets();
		semantics();
	}
	tz::gl::vk2::terminate();
	tz::terminate();
}
