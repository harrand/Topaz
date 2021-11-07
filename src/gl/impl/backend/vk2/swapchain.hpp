#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_SWAPCHAIN_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_SWAPCHAIN_HPP
#include "gl/impl/backend/vk2/logical_device.hpp"
#include "gl/impl/backend/vk2/image_view.hpp"

namespace tz::gl::vk2
{
	/**
	 * @ingroup tz_gl_vk_presentation
	 * Specifies parameters of a newly created @ref Swapchain.
	 */
	struct SwapchainInfo
	{
		/// Pointer to a valid @ref LogicalDevice. This must not be null.
		const LogicalDevice* device;
		/// Pointer to a valid @ref WindowSurface onto which the Swapchain shall present images. This must not be null.
		const WindowSurface* surface;
		/// Minimum number of swapchain images. This must be satisfied by the @ref PhysicalDevice corresponding to `device.get_hardware()`. See @ref PhysicalDevice::get_surface_capabilities for more information. 
		std::uint32_t swapchain_image_count_minimum;
		/// Specifies the format the swapchain image(s) will be created with. 
		ImageFormat image_format;
		// sRGB colourspace hardcoded.
		// Specifies the presentation mode the @ref Swapchain will use. This must be satisfied by the @ref PhysicalDevice corresponding to `device.get_hardware()`. See @ref PhysicalDevice::get_supported_surface_present_modes for more information.
		SurfacePresentMode present_mode;
	};

	/**
	 * @ingroup tz_gl_vk_presentation
	 * Swapchains are infrastructures which represent GPU images we will render to before they can be presented to the screen.
	 */
	class Swapchain
	{
	public:
		/**
		 * Construct a new Swapchain.
		 */
		Swapchain(SwapchainInfo info);
		Swapchain(const Swapchain& copy) = delete;
		Swapchain(Swapchain&& move);
		~Swapchain();
		Swapchain& operator=(const Swapchain& rhs) = delete;
		Swapchain& operator=(Swapchain&& rhs);

		const LogicalDevice& get_device() const;

		/**
		 * Create a Swapchain which doesn't do anything.
		 * @note It is an error to use null Swapchains for most operations. Retrieving the native  handle and querying for null-ness are the only valid operations on a null Swapchain.
		 */
		static Swapchain null();
		/**
		 * Query as to whether the Swapchain is null. Null Swapchains cannot perform operations or be used for @ref WindowSurface presentation.
		 * See @ref Swapchain::null for more information.
		 */
		bool is_null() const;
		using NativeType = VkSwapchainKHR;
		NativeType native() const;
		/**
		 * Retrieve a span of all presentable images associated with this Swapchain.
		 * @return Span containing all swapchain images.
		 */
		std::span<const Image> get_images() const;
		std::span<const ImageView> get_image_views() const;
		ImageFormat get_image_format() const;
		Vec2ui get_dimensions() const;
	private:
		Swapchain();
		void initialise_images();

		VkSwapchainKHR swapchain;
		SwapchainInfo info;
		Vec2ui dimensions;
		std::vector<Image> swapchain_images;
		std::vector<ImageView> swapchain_image_views;
	};
}

#endif // TOPAZ_GL_IMPL_BACKEND_VK2_SWAPCHAIN_HPP
