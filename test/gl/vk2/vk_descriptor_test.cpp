#include "gl/impl/backend/vk2/descriptors.hpp"

void basic_descriptor_layouts()
{
	using namespace tz::gl::vk2;
	PhysicalDevice pdev = get_all_devices().front();
	LogicalDeviceInfo linfo;
	linfo.physical_device = pdev;

	LogicalDevice ldev{linfo};
	{
		constexpr std::uint32_t low_count = 1;
		constexpr std::uint32_t medium_count = 8;
		constexpr std::uint32_t high_count = 128;
		constexpr std::uint32_t huge_count = 512 * 512;
		DescriptorLayoutBuilder empty, some, lots;
		empty.set_device(ldev); some.set_device(ldev); lots.set_device(ldev);
		
		some.with_binding({.type = DescriptorType::StorageBuffer, .count = low_count})
		    .with_binding({.type = DescriptorType::ImageWithSampler, .count = high_count});

		lots.with_binding({.type = DescriptorType::StorageBuffer, .count = medium_count})
		    .with_binding({.type = DescriptorType::StorageImage, .count = medium_count})
		    .with_binding({.type = DescriptorType::ImageWithSampler, .count = huge_count})
		    .with_binding({.type = DescriptorType::StorageImage, .count = medium_count});

		DescriptorLayout empty_layout = empty.build();
		DescriptorLayout some_layout = some.build();
		DescriptorLayout lots_layout = lots.build();

		// Ensure 'empty' makes sense.
		// - Nothing in the layout
		tz_assert(empty_layout.descriptor_count() == 0, "Empty DescriptorLayout had %zu descriptors, expected %d", empty_layout.descriptor_count(), 0);
		tz_assert(empty_layout.binding_count() == 0, "Empty DescriptorLayout had %zu bindings, expected %d", empty_layout.binding_count(), 0);

		
		// Ensure 'some' makes sense.
		// We expect: 2 bindings
		// - Low number of StorageBuffers
		// - High number of ImageWithSamplers
		tz_assert(some_layout.binding_count() == 2, "DescriptorLayout 'some' had %zu bindings, expected %d", some_layout.binding_count(), 2);
		tz_assert(some_layout.descriptor_count() == low_count + high_count, "DescriptorLayout 'some' had %zu descriptors, expecteed %u", some_layout.descriptor_count(), low_count + high_count);
		tz_assert(some_layout.descriptor_count_of(DescriptorType::StorageBuffer) == low_count, "DescriptorLayout 'some' had %zu StorageBuffers, expected %u", some_layout.descriptor_count_of(DescriptorType::StorageBuffer), low_count);
		tz_assert(some_layout.descriptor_count_of(DescriptorType::ImageWithSampler) == high_count, "DescriptorLayout 'some' had %zu StorageBuffers, expected %u", some_layout.descriptor_count_of(DescriptorType::ImageWithSampler), high_count);

		// Ensure 'lots' makes sense.
		// We expect: 4 bindings
		// - Medium number of StorageBuffers
		// - 2x Medium number of StorageImages
		// - Huge number of ImageWithSamplers
		tz_assert(lots_layout.binding_count() == 4, "DescriptorLayout 'lots' had %zu bindings, expected %d", lots_layout.binding_count(), 4);
		tz_assert(lots_layout.descriptor_count() == medium_count + medium_count + huge_count + medium_count, "DescriptorLayout 'lots' had %zu descriptors, expected %u", lots_layout.descriptor_count(), medium_count + medium_count + huge_count + medium_count);
		tz_assert(lots_layout.descriptor_count_of(DescriptorType::StorageBuffer) == medium_count, "DescriptorLayout 'lots' had %zu StorageBuffers, expected %u", lots_layout.descriptor_count_of(DescriptorType::StorageBuffer), medium_count);
		tz_assert(lots_layout.descriptor_count_of(DescriptorType::StorageImage) == medium_count + medium_count, "DescriptorLayout 'lots' had %zu StorageImages, expected %u", lots_layout.descriptor_count_of(DescriptorType::StorageImage), medium_count + medium_count);
		tz_assert(lots_layout.descriptor_count_of(DescriptorType::ImageWithSampler) == huge_count, "DescriptorLayout 'lots' had %zu ImageWithSamplers, expected %u", lots_layout.descriptor_count_of(DescriptorType::ImageWithSampler), huge_count);
	}
}

void descriptor_layouts_descriptor_indexing()
{
	using namespace tz::gl::vk2;
	PhysicalDevice pdev = get_all_devices().front();
	if(!pdev.get_supported_features().contains(DeviceFeature::BindlessDescriptors))
	{
		// If the PhysicalDevice doesn't support bindless, we can't possibly test it.
		return;
	}
	LogicalDeviceInfo linfo;
	linfo.physical_device = pdev;
	linfo.features = {DeviceFeature::BindlessDescriptors};

	LogicalDevice ldev{linfo};
	{
		// Create a layout but using the DescriptorFlags corresponding to VK_descriptor_indexing_KHR
		DescriptorLayoutBuilder builder;
		builder.set_device(ldev);
		builder.with_binding({.type = DescriptorType::ImageWithSampler,
				    .count = 512,
				    .flags = {DescriptorFlag::UpdateAfterBind,
				    	      DescriptorFlag::UpdateUnusedWhilePending,
					      DescriptorFlag::PartiallyBound,
					      DescriptorFlag::VariableCount}});
		DescriptorLayout layout = builder.build();
	}
}

void basic_descriptor_pools()
{
	using namespace tz::gl::vk2;
	PhysicalDevice pdev = get_all_devices().front();
	LogicalDeviceInfo linfo;
	linfo.physical_device = pdev;

	LogicalDevice ldev{linfo};
	{
		DescriptorLayoutBuilder builder;
		builder.set_device(ldev);
		builder.with_binding({.type = DescriptorType::StorageBuffer, .count = 1})
		       .with_binding({.type = DescriptorType::ImageWithSampler, .count = 3});

		DescriptorLayout layout = builder.build();
		// Let's create a pool enough for one set of this layout, and another for two.
		DescriptorPoolInfo pool_for_one = DescriptorPoolInfo::to_fit_layout(layout, 1);
		DescriptorPoolInfo pool_for_two = DescriptorPoolInfo::to_fit_layout(layout, 2);

		DescriptorPool p1{pool_for_one};
		DescriptorPool p2{pool_for_two};
	}
}

void descriptor_pools_descriptor_indexing()
{
	using namespace tz::gl::vk2;
	PhysicalDevice pdev = get_all_devices().front();
	if(!pdev.get_supported_features().contains(DeviceFeature::BindlessDescriptors))
	{
		return;
	}
	LogicalDeviceInfo linfo;
	linfo.physical_device = pdev;
	linfo.features = {DeviceFeature::BindlessDescriptors};

	LogicalDevice ldev{linfo};
	{
		DescriptorLayoutBuilder builder;
		builder.set_device(ldev);
		builder.with_binding({.type = DescriptorType::ImageWithSampler, .count = 3})
	 	       .with_binding({.type = DescriptorType::StorageBuffer, .count = 1,
					.flags = {DescriptorFlag::UpdateAfterBind,
						  DescriptorFlag::UpdateUnusedWhilePending,
						  DescriptorFlag::PartiallyBound,
						  DescriptorFlag::VariableCount}});
		DescriptorLayout layout = builder.build();
		// Let's create a pool enough for one set of this layout, and ensure validation layers don't complain about the descriptor-indexing flags.
		DescriptorPoolInfo pool_info = DescriptorPoolInfo::to_fit_layout(layout, 1);

		DescriptorPool pool{pool_info};
	}
}

void basic_descriptor_set_allocation()
{
	using namespace tz::gl::vk2;
	PhysicalDevice pdev = get_all_devices().front();
	LogicalDeviceInfo linfo;
	linfo.physical_device = pdev;

	LogicalDevice ldev{linfo};
	{
		DescriptorLayoutBuilder builder;
		builder.set_device(ldev);
		builder.with_binding({.type = DescriptorType::StorageBuffer, .count = 1})
		       .with_binding({.type = DescriptorType::ImageWithSampler, .count = 3});

		DescriptorLayout layout = builder.build();
		// Let's create a pool enough for one set of this layout, and another for two.
		DescriptorPoolInfo pinfo = DescriptorPoolInfo::to_fit_layout(layout, 1);

		DescriptorPool pool{pinfo};
		DescriptorPool::Allocation alloc
		{
			.set_layouts = {&layout}
		};
		DescriptorPool::AllocationResult alloc_res = pool.allocate_sets(alloc);
		tz_assert(alloc_res.success(), "DescriptorPool allocation failed unexpectedly.");
		tz_assert(alloc_res.sets.length() == 1, "DescriptorPool allocation returned list of sets of unexpected size. Expected %d, got %zu", 1, alloc_res.sets.length());
		DescriptorSet set = alloc_res.sets.front();
	}
}

void descriptor_set_allocation_descriptor_indexing()
{
	using namespace tz::gl::vk2;
	PhysicalDevice pdev = get_all_devices().front();
	if(!pdev.get_supported_features().contains(DeviceFeature::BindlessDescriptors))
	{
		return;
	}
	LogicalDeviceInfo linfo;
	linfo.physical_device = pdev;
	linfo.features = {DeviceFeature::BindlessDescriptors};

	LogicalDevice ldev{linfo};
	{
		DescriptorLayoutBuilder builder;
		builder.set_device(ldev);
		builder.with_binding({.type = DescriptorType::ImageWithSampler, .count = 3})
	 	       .with_binding({.type = DescriptorType::StorageBuffer, .count = 1,
					.flags = {DescriptorFlag::UpdateAfterBind,
						  DescriptorFlag::UpdateUnusedWhilePending,
						  DescriptorFlag::PartiallyBound,
						  DescriptorFlag::VariableCount}});
		DescriptorLayout layout = builder.build();
		// Let's create a pool enough for one set of this layout, and ensure validation layers don't complain about the descriptor-indexing flags.
		DescriptorPoolInfo pool_info = DescriptorPoolInfo::to_fit_layout(layout, 1);

		DescriptorPool pool{pool_info};
		// Let's create one set.
		DescriptorPool::Allocation alloc
		{
			.set_layouts = {&layout}
		};
		DescriptorPool::AllocationResult alloc_res = pool.allocate_sets(alloc);
		tz_assert(alloc_res.success(), "DescriptorPool allocation failed unexpectedly.");
		tz_assert(alloc_res.sets.length() == 1, "DescriptorPool allocation returned list of sets of unexpected size. Expected %d, got %zu", 1, alloc_res.sets.length());
		DescriptorSet set = alloc_res.sets.front();
	}
}

int main()
{
	tz::GameInfo game{"vk_descriptor_test", tz::Version{1, 0, 0}, tz::info()};
	tz::initialise(game, tz::ApplicationType::Headless);
	tz::gl::vk2::initialise(game, tz::ApplicationType::Headless);
	{
		basic_descriptor_layouts();
		descriptor_layouts_descriptor_indexing();
		basic_descriptor_pools();
		descriptor_pools_descriptor_indexing();

		basic_descriptor_set_allocation();
		descriptor_set_allocation_descriptor_indexing();
	}
	tz::gl::vk2::terminate();
	tz::terminate();
}
