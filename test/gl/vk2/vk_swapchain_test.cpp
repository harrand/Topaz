#include "core/types.hpp"
#include "gl/impl/backend/vk2/extensions.hpp"
#include "gl/impl/backend/vk2/hardware/physical_device.hpp"
#include "gl/impl/backend/vk2/swapchain.hpp"

tz::gl::vk2::SwapchainInfo get_safe_swapchain_info(const tz::gl::vk2::LogicalDevice& ldev, const tz::gl::vk2::WindowSurface& window_surf)
{
	using namespace tz::gl::vk2;
	const PhysicalDevice& pdev = ldev.get_hardware();
	SwapchainInfo sinfo;
	sinfo.device = &ldev;
	sinfo.surface = &window_surf;
	// The next params need to be supported by the PhysicalDevice, so we'll make it any old thing that the PhysicalDevice supports.
	// We start with an image count minimum.
	PhysicalDeviceSurfaceCapabilityInfo pdev_surface_cap = pdev.get_surface_capabilities(window_surf);	
	sinfo.swapchain_image_count_minimum = pdev_surface_cap.min_image_count;
	// Now an image format.
	tz::BasicList<ImageFormat> fmt_list = pdev.get_supported_surface_formats(window_surf);
	tz_assert(!fmt_list.empty(), "Tried to create a Swapchain from a PhysicalDevice which doesn't support any surface image formats. vk_physical_device_test probably fails too. If not, this test needs changes to more sensibly choose a PhysicalDevice.");
	sinfo.image_format = fmt_list.front();
	// And finally, a present mode.	
	tz::BasicList<SurfacePresentMode> pmode_list = pdev.get_supported_surface_present_modes(window_surf);
	tz_assert(!pmode_list.empty(), "Tried to create a Swapchain from a PhysicalDevice which doesn't support any surface present modes. vk_physical_device_test probably fails too. If not, this test needs changes to more sensibly choose a PhysicalDevice");
	sinfo.present_mode = pmode_list.front();
	return sinfo;
}

void mandatory_swapchain(tz::GameInfo g)
{
	// No shared state, create our own instance and everything...
	// Create a swapchain with settings that are mandatory for all vulkan driver implementations, and make sure nothing crashes.
	using namespace tz::gl::vk2;
	VulkanInfo info{g, InstanceExtensionList{InstanceExtension::DebugMessenger}};
	VulkanInstance window_inst{info, tz::ApplicationType::HiddenWindowApplication};
	PhysicalDeviceList devices = get_all_devices(window_inst);	
	PhysicalDevice pdev = devices.front();
	WindowSurface window_surf{window_inst, tz::window()};

	// Create a LogicalDevice, and then use that to create a Swapchain.
	LogicalDeviceInfo linfo;
	linfo.physical_device = pdev;
	linfo.extensions = DeviceExtensionList{DeviceExtension::Swapchain};
	linfo.surface = &window_surf;
	LogicalDevice ldev{linfo};

	SwapchainInfo sinfo = get_safe_swapchain_info(ldev, window_surf);

	// Now we create the Swapchain! Finally...
	Swapchain swapchain{sinfo};

	// Let's check the number of swapchain images fits what the PhysicalDevice required (this should 100% happen).
	PhysicalDeviceSurfaceCapabilityInfo pdev_surface_cap = pdev.get_surface_capabilities(window_surf);
	std::span<const Image> swapchain_images = swapchain.get_images();
	tz_assert(swapchain_images.size() >= pdev_surface_cap.min_image_count, "Swapchain::get_images() returned an ImageSpan of size %zu which is smaller than the PhysicalDevice minimum image count of %u", swapchain_images.size(), pdev_surface_cap.min_image_count);
	tz_assert(swapchain_images.size() <= pdev_surface_cap.max_image_count, "Swapchain::get_images() returned an ImageSpan of size %zu which is larger than the PhysicalDevice maximum image count of %u", swapchain_images.size(), pdev_surface_cap.max_image_count);

	ImageFormat swapchain_image_fmt = swapchain.get_image_format();
	bool all_same_format = std::all_of(swapchain_images.begin(), swapchain_images.end(), [swapchain_image_fmt](const Image& img)
	{
		return img.get_format() == swapchain_image_fmt && img.get_format() != ImageFormat::Undefined;
	});
	tz_assert(all_same_format, "Swapchain images did not all have the same format (or they were ImageFormat::Undefined)");

	constexpr ImageLayout initial_swapchain_image_layout = ImageLayout::Undefined;
	bool all_correct_layout = std::all_of(swapchain_images.begin(), swapchain_images.end(), [initial_swapchain_image_layout](const Image& img)
	{
		return img.get_layout() == initial_swapchain_image_layout;
	});
	tz_assert(all_correct_layout, "Swapchain images did not all have the expected ImageLayout::Undefined. The spec demands it!");

	const tz::Vec2ui dims = swapchain.get_dimensions();
	bool all_correct_size = std::all_of(swapchain_images.begin(), swapchain_images.end(), [&dims](const Image& img)
	{
		return img.get_dimensions() == dims;
	});
	tz_assert(all_correct_size, "Swapchain images did not all have the expected size of {%u, %u}", dims[0], dims[1]);
}

void swapchain_extension_supported()
{
	using namespace tz::gl::vk2;
	PhysicalDeviceList pdevs = get_all_devices();
	bool anyone_supports_swapchain = std::any_of(pdevs.begin(), pdevs.end(), [](const PhysicalDevice& pdev)
	{
		return pdev.get_supported_extensions().contains(DeviceExtension::Swapchain);
	});
	tz_assert(anyone_supports_swapchain, "In a non-headless application, no PhysicalDevices were found which support DeviceExtension::Swapchain");
}

void semantics()
{
	using namespace tz::gl::vk2;	
	// Ensure constexpr stuff
	static_assert(!tz::copyable<Swapchain>, "Swapchain is wrongly copyable");
	static_assert(tz::moveable<Swapchain>, "Swapchain is wrongly not moveable");
	// Ensure move doesn't assert/crash:
	// We will need a valid PhysicalDevice.
	tz_assert(!get_all_devices().empty(), "No valid PhysicalDevices");
	LogicalDeviceInfo dummy;
	dummy.physical_device = get_all_devices().front();
	dummy.extensions = DeviceExtensionList{DeviceExtension::Swapchain};
	dummy.surface = &get_window_surface();
	{
		LogicalDevice ldev{dummy};

		SwapchainInfo sinfo = get_safe_swapchain_info(ldev, get_window_surface());
		Swapchain s1{sinfo};
		tz_assert(!s1.is_null(), "Swapchain wrongly thinks its null");
		Swapchain s2{std::move(s1)}; // s1 dies
		tz_assert(!s2.is_null(), "Swapchain wrongly thinks its null");
		Swapchain s3{Swapchain::null()};
		tz_assert(s3.is_null(), "Swapchain wrongly thinks its non-null");
		s3 = std::move(s2); // s2 dies
		tz_assert(!s3.is_null(), "Swapchain wrongly thinks its null");
	}
}

int main()
{
	tz::GameInfo g{"vk_swapchain_test", tz::Version{1, 0, 0}, tz::info()};
	tz::initialise(g, tz::ApplicationType::HiddenWindowApplication);
	tz::gl::vk2::initialise(g, tz::ApplicationType::HiddenWindowApplication);
	{
		swapchain_extension_supported();
		mandatory_swapchain(g);
		semantics();
	}
	tz::gl::vk2::terminate();
	tz::terminate();
}
