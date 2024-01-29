#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_SWAPCHAIN_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_SWAPCHAIN_HPP
#if TZ_VULKAN
#include "tz/gl/impl/vulkan/detail/logical_device.hpp"
#include "tz/gl/impl/vulkan/detail/image_view.hpp"

namespace tz::gl::vk2
{
	class Swapchain;
	/**
	 * @ingroup tz_gl_vk_presentation
	 * Specifies parameters of a newly created @ref Swapchain.
	 */
	struct SwapchainInfo
	{
		/// Pointer to a valid @ref LogicalDevice. This must not be null.
		const LogicalDevice* device;
		/// Minimum number of swapchain images. This must be satisfied by the @ref PhysicalDevice corresponding to `device.get_hardware()`. See @ref PhysicalDevice::get_surface_capabilities for more information. 
		std::uint32_t swapchain_image_count_minimum;
		/// Specifies the format the swapchain image(s) will be created with. 
		image_format format;
		/// Specifies the presentation mode the @ref Swapchain will use. This must be satisfied by the @ref PhysicalDevice corresponding to `device.get_hardware()`. See @ref PhysicalDevice::get_supported_surface_present_modes for more information.
		SurfacePresentMode present_mode;
		/// Pointer to swapchain which this will replace, if any. Defaults to nullptr.
		const Swapchain* old_swapchain = nullptr;
	};

	/**
	 * @ingroup tz_gl_vk_presentation
	 * Swapchains are infrastructures which represent GPU images we will render to before they can be presented to the screen.
	 */
	class Swapchain
	{
	public:
		/**
		 * Specify information about a request to acquire the next available presentable image.
		 * See @ref Swapchain::acquire_image for usage.
		 */
		struct ImageAcquisition
		{
			/// Semaphore to signal when the image has been retrieved. If not required, this can be nullptr.
			const BinarySemaphore* signal_semaphore = nullptr;
			/// Fence to signal when the image has been retrieved. If not required, this can be nullptr.
			const Fence* signal_fence = nullptr;
			/// Specifies number of nanoseconds which can pass before failing if no presentable images are available. By default, this is forever.
			std::uint64_t timeout = std::numeric_limits<std::uint64_t>::max();
		};

		struct ImageAcquisitionResult
		{
			enum class AcquisitionResultType
			{
				AcquireSuccess,
				Suboptimal,
				ErrorOutOfDate,
				ErrorSurfaceLost,
				ErrorUnknown
			};
			std::uint32_t image_index;
			AcquisitionResultType type;
		};
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
		 * Retrieve a presentable image index.
		 * See @ref ImageAcquisition for details.
		 */
		ImageAcquisitionResult acquire_image(const ImageAcquisition& acquire);

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
		/**
		 * Retrieve a span of all presentable images associated with this Swapchain.
		 * @return Span containing all swapchain images.
		 */
		std::span<Image> get_images();
		/**
		 * Retrieve a span of all ImageViews corresponding to an image associated with this Swapchain.
		 * `this->get_image_views()[i].get_image() == this->get_images()[i]` is guaranteed to be the case.
		 */
		std::span<const ImageView> get_image_views() const;
		/**
		 * Retrieve a span of all ImageViews corresponding to an image associated with this Swapchain.
		 * `this->get_image_views()[i].get_image() == this->get_images()[i]` is guaranteed to be the case.
		 */
		std::span<ImageView> get_image_views();
		/**
		 * Retrieve the image_format of the swapchain images.
		 * @return Swapchain format.
		 */
		image_format get_image_format() const;
		/**
		 * Retrieve the present mode used by the swapchain.
		 * @return Present mode for the window surface.
		 */
		SurfacePresentMode get_present_mode() const;
		/**
		 * Retrieve the dimensions of the swapchain images.
		 * @return Swapchain image dimensions, in pixels.
		 */
		tz::vec2ui get_dimensions() const;
		void refresh();
		void refresh(SurfacePresentMode new_present_mode);
	private:
		Swapchain();
		void initialise_images();

		VkSwapchainKHR swapchain;
		SwapchainInfo info;
		tz::vec2ui dimensions;
		std::vector<Image> swapchain_images;
		std::vector<ImageView> swapchain_image_views;
	};
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_SWAPCHAIN_HPP