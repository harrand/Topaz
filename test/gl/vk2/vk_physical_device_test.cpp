#include "core/tz.hpp"
#include "core/types.hpp"
#include "gl/impl/backend/vk2/features.hpp"
#include "gl/impl/backend/vk2/hardware/physical_device.hpp"
#include "gl/impl/backend/vk2/image_format.hpp"

namespace drivers
{
	// vk2::PhysicalDevice surrounds its VkPhysicalDevice native handle.
	// We want to be able to work with a physical device, with or without given features.
	// There is no guarantee such devices are available on the running machine, so we use a driver to mimic the results such devices would provide. An example of this would be device features such as MDI.
	constexpr tz::gl::vk2::detail::DeviceFeatureInfo mdi(bool avail)
	{
		tz::gl::vk2::detail::DeviceFeatureInfo ret;
		ret.features.features.multiDrawIndirect = avail ? VK_TRUE : VK_FALSE;
		return ret;
	}

	tz::gl::vk2::DeviceExtensionList swapchain(bool avail)
	{
		tz::gl::vk2::DeviceExtensionList l;
		if(avail)
		{
			l  |= tz::gl::vk2::DeviceExtension::Swapchain;
		}
		return l;
	}
}

void devices()
{
	using namespace tz::gl::vk2;
	PhysicalDeviceList devices = get_all_devices();	

	tz_assert(!devices.empty(), "No physical devices are available");
	tz_assert(devices.front().native() != VK_NULL_HANDLE, "get_all_devices() returns nonsense");

	// Ensure that no devices have invalid extensions.
	for(const PhysicalDevice& dev : devices)
	{
		for(DeviceExtension e : dev.get_supported_extensions())
		{
			tz_assert(e != DeviceExtension::Count, "PhysicalDevice list of supported extensions includes Extension::Count, which makes no sense.");
		}
	}

	// We'll play around with all the supported features.
	// MDI
	DeviceFeatureField mdi_yes = detail::to_feature_field(drivers::mdi(true));
	DeviceFeatureField mdi_no = detail::to_feature_field(drivers::mdi(false));

	tz_assert(mdi_yes.contains(DeviceFeature::MultiDrawIndirect), "PhysicalDeviceFeatureField::MultiDrawIndirect (MDI) is not handled properly.");
	tz_assert(!mdi_no.contains(DeviceFeature::MultiDrawIndirect), "PhysicalDeviceFeatureField::MultiDrawIndirect (MDI) is not handled properly.");

	// Now we'll try some supported extensions.
	DeviceExtensionList swapchain_yes = drivers::swapchain(true);
	DeviceExtensionList swapchain_no = drivers::swapchain(false);

	tz_assert(swapchain_yes.contains(DeviceExtension::Swapchain), "ExtensionList wrongly says it doesn't Extension::Swapchain");
	tz_assert(!swapchain_no.contains(DeviceExtension::Swapchain), "ExtensionList wrongly says it contains Extension::Swapchain");

}

void guaranteed_formats()
{
	using namespace tz::gl::vk2;
	PhysicalDeviceList devices = get_all_devices();	

	// Ensure all PhysicalDevices support the guaranteed formats.
	for(const PhysicalDevice& dev : devices)
	{
		using ImageFormats = std::span<const ImageFormat>;
		constexpr ImageFormats cols = format_traits::get_mandatory_colour_attachment_formats();
		constexpr ImageFormats depths = format_traits::get_mandatory_depth_attachment_formats();
		constexpr ImageFormats sampleds = format_traits::get_mandatory_sampled_image_formats();

		bool cols_satisfied = std::all_of(cols.begin(), cols.end(), [&dev](const ImageFormat& fmt)
		{
			return dev.supports_image_colour_format(fmt);
		});

		bool depths_satisfied = std::all_of(depths.begin(), depths.end(), [&dev](const ImageFormat& fmt)
		{
			return dev.supports_image_depth_format(fmt);
		});

		bool sampleds_satisfied = std::all_of(sampleds.begin(), sampleds.end(), [&dev](const ImageFormat& fmt)
		{
			return dev.supports_image_sampled_format(fmt);
		});

		tz_assert(cols_satisfied, "Mandatory ImageFormats not available on this PhysicalDevice (Colour Attachment)");
		tz_assert(depths_satisfied, "Mandatory ImageFormats not available on this PhysicalDevice (Depth Attachment)");
		tz_assert(sampleds_satisfied, "Mandatory ImageFormats not available on this PhysicalDevice (Sampled Image)");
	}
}

void window_surfaces(tz::GameInfo game)
{
	using namespace tz::gl::vk2;
	VulkanInstance window_inst
	{{
		.game_info = game,
		.app_type = tz::ApplicationType::HiddenWindowApplication
	}};
	PhysicalDeviceList devices = get_all_devices(window_inst);	
	WindowSurface window_surf{window_inst, tz::window()};
	// If we have a PhysicalDevice which supports WindowSurfaces, ensure that its swapchain format support isn't shit
	for(const PhysicalDevice& dev : devices)
	{
		tz::BasicList<ImageFormat> supported_formats = dev.get_supported_surface_formats(window_surf);
		tz_assert(!supported_formats.empty(), "PhysicalDevice found which doesn't support any window surface formats");
	}
}

void surface_present_modes(tz::GameInfo game)
{
	using namespace tz::gl::vk2;
	VulkanInstance window_inst
	{{
		.game_info = game,
		.app_type = tz::ApplicationType::HiddenWindowApplication
	}};
	PhysicalDeviceList devices = get_all_devices(window_inst);	
	WindowSurface window_surf{window_inst, tz::window()};

	// Ensure all PhysicalDevices support the guaranteed formats.
	for(const PhysicalDevice& dev : devices)
	{
		tz::BasicList<SurfacePresentMode> supported_present_modes = dev.get_supported_surface_present_modes(window_surf);
		for(SurfacePresentMode mandatory_present_mode : present_traits::get_mandatory_present_modes())
		{
			tz_assert(supported_present_modes.contains(mandatory_present_mode), "PhysicalDevice found which doesn't support the mandatory surface present modes.");
		}
	}
}

void semantics()
{
	using namespace tz::gl::vk2;
	static_assert(tz::copyable<PhysicalDevice>, "PhysicalDevice is wrongly non-copyable");	
	static_assert(tz::moveable<PhysicalDevice>, "PhysicalDevice is wrongly non-moveable");	
}

int main()
{
	tz::GameInfo game{"vk_physical_device_test", tz::Version{1, 0, 0}, tz::info()};
	tz::ApplicationType app_type = tz::ApplicationType::HiddenWindowApplication;
	tz::initialise(game, app_type);
	// TODO: Don't need to specifically vk2::initialise/term once vk2 is hooked up to tz::initialise.
	tz::gl::vk2::initialise(game, app_type);
	{
		devices();
		guaranteed_formats();
		window_surfaces(game);
		surface_present_modes(game);
		semantics();
	}
	tz::gl::vk2::terminate();
	tz::terminate();
}
