#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_SWAPCHAIN_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_SWAPCHAIN_HPP
#include "gl/impl/backend/vk2/logical_device.hpp"

namespace tz::gl::vk2
{
	struct SwapchainInfo
	{
		const LogicalDevice* device;
		const WindowSurface* surface;
		std::uint32_t swapchain_image_count_minimum;
		ImageFormat image_format;
		// sRGB colourspace hardcoded.
		SurfacePresentMode present_mode;
	};

	class Swapchain
	{
	public:
		Swapchain(SwapchainInfo info);
		Swapchain(const Swapchain& copy) = delete;
		Swapchain(Swapchain&& move);
		~Swapchain();
		Swapchain& operator=(const Swapchain& rhs) = delete;
		Swapchain& operator=(Swapchain&& rhs);

		static Swapchain null();
		bool is_null() const;
	private:
		Swapchain();

		VkSwapchainKHR swapchain;
		SwapchainInfo info;
	};
}

#endif // TOPAZ_GL_IMPL_BACKEND_VK2_SWAPCHAIN_HPP
