#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_TZ_VULKAN_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_TZ_VULKAN_HPP
#if TZ_VULKAN
#include "core/window.hpp"
#include "core/version.hpp"
#include "core/game_info.hpp"
#include "core/tz.hpp"
#include "gl/impl/backend/vk2/extensions.hpp"
#include "vk_mem_alloc.h"
#include <cstdint>
namespace tz::gl::vk2
{
	class VulkanInstance;

	/**
	 * @ingroup tz_gl2_graphicsapi_vk
	 * @defgroup tz_gl_vk Vulkan Backend
	 * Documentation for the Vulkan Backend, which acts as a large abstraction around the Vulkan API. This is a low-level module free of all context relevant to Topaz.
	 */

	/**
	 * @ingroup tz_gl_vk
	 * @defgroup tz_gl_vk_presentation Presentation and Window Surface Interation (WSI)
	 * Documentation for functionality related to presenting images to existing windows.
	 */

	/**
	 * @ingroup tz_gl_vk
	 * @defgroup tz_gl_vk_sync Synchronisation Primitives
	 * Documentation for functionality related to CPU/GPU Vulkan synchronisation primitives.
	 */

	/**
	 * @ingroup tz_gl_vk
	 * @defgroup tz_gl_vk_extension Extensions and Features
	 * Documentation for functionality related to instance/device extensions, and optional features.
	 */

	/**
	 * @ingroup tz_gl_vk
	 * @defgroup tz_gl_vk_image Images, Samplers and Formats
	 * Documentation for everything relating to Images, Samplers and ImageFormats.
	 */

	/**
	 * @ingroup tz_gl_vk
	 * @defgroup tz_gl_vk_buffer Buffers
	 * Documentation for Vulkan Buffers.
	 */

	/**
	 * @ingroup tz_gl_vk
	 * @defgroup tz_gl_vk_descriptors Descriptor Layouts and Sets
	 * Documentation for everything related to Descriptors.
	 */

	/**
	 * @ingroup tz_gl_vk
	 * @defgroup tz_gl_vk_graphics_pipeline Graphics Pipeline
	 * Documentation for the graphics pipeline.
	 */
	
		/**
		 * @ingroup tz_gl_vk_graphics_pipeline
		 * @defgroup tz_gl_vk_graphics_pipeline_shader Shader Programs and Modules
		 * Documentation for shaders.
		 */

		/**
		 * @ingroup tz_gl_vk_graphics_pipeline
		 * @defgroup tz_gl_vk_graphics_pipeline_fixed Fixed Pipeline State Values
		 * Documentation for the fixed-pipeline settings, such as rasteriser state and input assembly.
		 */

		/**
		 * @ingroup tz_gl_vk_graphics_pipeline
		 * @defgroup tz_gl_vk_graphics_pipeline_render_pass Render Passes
		 * Documentation for render passes and how they interact with the rendering pipeline.
		 */
	
	/**
	 * @ingroup tz_gl_vk
	 * @defgroup tz_gl_vk_commands Command Buffers and Pools
	 * Documentation for everything related to Command Buffers, Command Pools and Vulkan Commands.
	 */

	/**
	 * @ingroup tz_gl_vk
	 * @{
	 */

	/**
	 * @brief Initialise the vulkan backend.
	 * @note This is intended to be invoked automatically by @ref tz::initialise however this is not yet implemented - You should invoke this yourself at the start of your program directly after `tz::initialise`
	 */
	void initialise(tz::GameInfo game_info, tz::ApplicationType app_type);
	/**
	 * @brief Terminate the vulkan backend
	 */
	void terminate();

	/**
	 * @brief Retrieve a reference to the current Vulkan Instance. This will have been created during initialisation.
	 */
	const VulkanInstance& get();
	bool is_headless();

	/**
	 * @}
	 */

	constexpr tz::Version vulkan_version{1, 2, 0};

	namespace util
	{
		using VkVersion = std::uint32_t;
		/**
		 * Convert a tz::Version to a VkVersion, which is used when interacting with the Vulkan API.
		 * @param ver Topaz version to convert
		 * @return Corresponding VkVersion.
		 */
		constexpr VkVersion tz_to_vk_version(tz::Version ver)
		{
			return VK_MAKE_VERSION(ver.major, ver.minor, ver.patch);
		}
	}

	class VulkanInstance;

	class VulkanDebugMessenger
	{
	public:
		static constexpr char debug_message_shader_printf_label[] = "TZ_ASSERT_SHADER";
		VulkanDebugMessenger(const VulkanInstance& instance);
		VulkanDebugMessenger(const VulkanDebugMessenger& copy) = delete;
		VulkanDebugMessenger(VulkanDebugMessenger&& move);
		~VulkanDebugMessenger();

		VulkanDebugMessenger& operator=(const VulkanDebugMessenger& rhs) = delete;
		VulkanDebugMessenger& operator=(VulkanDebugMessenger&& rhs);
	private:
		VkDebugUtilsMessengerEXT debug_messenger;	
		VkInstance instance;
	};

	/**
	 * @ingroup tz_gl_vk_presentation
	 * Represents a Vulkan-friendly interface to an existing OS window. In order to present results to the screen, it is done via this WindowSurface class.
	 * @pre The @ref VulkanInstance associated with the surface must not be a Headless instance.
	 * @post Once a WindowSurface is created for a given @ref tz::Window, no other WindowSurfaces can be created for that window. This means only one VulkanInstance can own a Window.
	 */
	class WindowSurface
	{
	public:
		/**
		 * Create a WindowSurface for a given window via an existing VulkanInstance.
		 */
		WindowSurface(const VulkanInstance& instance, const tz::Window& window);	
		WindowSurface(const WindowSurface& copy) = delete;
		WindowSurface(WindowSurface&& move);
		~WindowSurface();

		WindowSurface& operator=(const WindowSurface& rhs) = delete;
		WindowSurface& operator=(WindowSurface&& rhs);

		const VulkanInstance& get_instance() const;
		const tz::Window& get_window() const;

		using NativeType = VkSurfaceKHR;
		NativeType native() const;
	private:
		VkSurfaceKHR surface;
		const VulkanInstance* instance;
		const tz::Window* window;
	};

	/**
	 * @ingroup tz_gl_vk
	 * Specifies creation flags for a @ref VulkanInstance.
	 */
	struct VulkanInstanceInfo
	{
		/// Information about the application.
		tz::GameInfo game_info;
		/// Describes the application type. If Headless is passed, the resultant @ref VulkanInstance will not have an associated @ref WindowSurface.
		tz::ApplicationType app_type = tz::ApplicationType::WindowApplication;
		/// List of extensions to enable. Empty by default.
		InstanceExtensionList extensions = {};
		/// Window from which to create a @ref WindowSurface. If nullptr is passed, no @ref WindowSurface is created. If `app_type == tz::ApplicationType::Headless`, this is ignored. Defaults to nullptr.
		const tz::Window* window = nullptr;
	};


	/**
	 * @ingroup tz_gl_vk
	 * Represents a vulkan instance, which acts as a reference to all per-application state.
	 * There is a default instance, retrievable via @ref vk2::get() but you can create additional instances.
	 */
	class VulkanInstance
	{
	public:
		VulkanInstance(VulkanInstanceInfo info);
		VulkanInstance(const VulkanInstance& copy) = delete;
		VulkanInstance(VulkanInstance&& move) = delete;
		~VulkanInstance();
		VulkanInstance& operator=(const VulkanInstance& rhs) = delete;
		VulkanInstance& operator=(VulkanInstance&& rhs) = delete;

		/**
		 * Query as to whether the instance is headless or not.
		 *
		 * A headless instance does not support WSI.
		 */
		bool is_headless() const;
		bool has_surface() const;

		const WindowSurface& get_surface() const;

		using NativeType = VkInstance;
		NativeType native() const;

		bool operator==(const VulkanInstance& rhs) const;
	private:
		VulkanInstanceInfo info;
		VkInstance instance;
		std::optional<VulkanDebugMessenger> maybe_debug_messenger;
		std::optional<WindowSurface> maybe_window_surface;
	};

	/**
	 * @ingroup tz_gl_vk_presentation
	 * Supported Presentation mode supported for a @ref WindowSurface.
	 */
	enum class SurfacePresentMode
	{
		/// - No internal queueing of presentation requests. Requests are applied instantly. Vulnerable to tearing.
		Immediate = VK_PRESENT_MODE_IMMEDIATE_KHR,
		/// - Recommended. Immune to tearing. If new requests come in before there is space for them, they will replace the old requests. No hard vsync.
		Mailbox = VK_PRESENT_MODE_MAILBOX_KHR,
		/// - Hard vsync. Immune to tearing. If new requests come in before there is space for them, the application will wait until there is space. Application will be locked to the FPS of the screen.
		Fifo = VK_PRESENT_MODE_FIFO_KHR,
		/// - Adaptive vsync. Vulnerable to tearing. If new requests come in too fast, existing requests are applied instantly, like Immediate.
		FifoRelaxed = VK_PRESENT_MODE_FIFO_RELAXED_KHR
	};

	constexpr SurfacePresentMode safe_present_modes[]
	{
		/* https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPresentModeKHR.html -> Description:
		 * VK_PRESENT_MODE_FIFO -- This is the only value of presentMode that is required to be supported. 
		 */
		SurfacePresentMode::Fifo
	};

	/**
	 * @ingroup tz_gl_vk_presentation
	 * Meta behaviour related to presentation and surfaces.
	 */
	namespace present_traits
	{
		/**
		 * Retrieve a span of all SurfacePresentModes which are guaranteed to be supported on any machine.
		 * You can safely use these in a @ref Swapchain without ensuring its corresponding @ref PhysicalDevice supports it via @ref PhysicalDevice::get_supported_surface_present_modes.
		 */
		constexpr std::span<const SurfacePresentMode> get_mandatory_present_modes()
		{
			return {safe_present_modes};
		}
	}
}
#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_TZ_VULKAN_HPP
