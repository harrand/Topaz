#include "core/tz.hpp"
#include "gl/impl/backend/vk2/tz_vulkan.hpp"
#include "gl/impl/backend/vk2/hardware/physical_device.hpp"

namespace drivers
{
	// vk2::PhysicalDevice surrounds its VkPhysicalDevice native handle.
	// We want to be able to work with a physical device, with or without given features.
	// There is no guarantee such devices are available on the running machine, so we use a driver to mimic the results such devices would provide. An example of this would be device features such as MDI.
	constexpr VkPhysicalDeviceFeatures mdi(bool avail)
	{
		VkPhysicalDeviceFeatures feat;
		feat.multiDrawIndirect = avail ? VK_TRUE : VK_FALSE;
		return feat;
	}
}

int main()
{
	tz::GameInfo game{"vk_physical_device_test", tz::Version{1, 0, 0}, tz::info()};
	tz::initialise(game, tz::ApplicationType::Headless);
	// TODO: Don't need to specifically vk2::initialise/term once vk2 is hooked up to tz::initialise.
	tz::gl::vk2::initialise_headless(game, tz::ApplicationType::Headless);
	{
		using namespace tz::gl::vk2;
		PhysicalDeviceList devices = get_all_devices();	

		tz_assert(!devices.empty(), "No physical devices are available");

		// We'll play around with all the supported features.
		// MDI
		PhysicalDeviceFeatureField mdi_yes = detail::to_feature_field(drivers::mdi(true));
		PhysicalDeviceFeatureField mdi_no = detail::to_feature_field(drivers::mdi(false));

		tz_assert(mdi_yes.contains(PhysicalDeviceFeature::MultiDrawIndirect), "PhysicalDeviceFeatureField::MultiDrawIndirect (MDI) is not handled properly.");
		tz_assert(!mdi_no.contains(PhysicalDeviceFeature::MultiDrawIndirect), "PhysicalDeviceFeatureField::MultiDrawIndirect (MDI) is not handled properly.");
	}
	tz::gl::vk2::terminate();
	tz::terminate();
}
