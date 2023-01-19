#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_TZ_VULKAN_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_TZ_VULKAN_HPP
#if TZ_VULKAN
#include "tz/wsi/window.hpp"
#include "hdk/data/version.hpp"
#include "tz/core/game_info.hpp"
#include "tz/tz.hpp"
#include "tz/gl/impl/vulkan/detail/extensions.hpp"
#include "vk_mem_alloc.h"
#include <cstdint>
#include <optional>

namespace tz::gl::vk2
{
	class VulkanInstance;

	/**
	 * @ingroup tz_gl2_graphicsapi_vk
	 * @defgroup tz_gl_vk Vulkan Backend
	 * Documentation for the Vulkan backend, which acts as a large abstraction around Vulkan. This is a low-level module free of all context relevant to Topaz.
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
	 * Documentation for everything relating to Images, Samplers and image_formats.
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
	void initialise(tz::game_info game_info);
	/**
	 * @brief Terminate the vulkan backend
	 */
	void terminate();

	/**
	 * @brief Retrieve a reference to the current Vulkan Instance. This will have been created during initialisation.
	 */
	const VulkanInstance& get();

	/**
	 * @}
	 */

	constexpr hdk::version vulkan_version{1, 2, 0, hdk::version_type::normal};

	namespace util
	{
		using VkVersion = std::uint32_t;
		/**
		 * Convert a hdk::version to a VkVersion, which is used when interacting with the Vulkan API.
		 * @param ver Topaz version to convert
		 * @return Corresponding VkVersion.
		 */
		constexpr VkVersion tz_to_vk_version(hdk::version ver)
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
	 * @post Once a WindowSurface is created for a given @ref tz::Window, no other WindowSurfaces can be created for that window. This means only one VulkanInstance can own a Window.
	 */
	class WindowSurface
	{
	public:
		/**
		 * Create a WindowSurface for a given window via an existing VulkanInstance.
		 */
		WindowSurface(const VulkanInstance& instance, const tz::wsi::window& window);	
		WindowSurface(const WindowSurface& copy) = delete;
		WindowSurface(WindowSurface&& move);
		~WindowSurface();

		WindowSurface& operator=(const WindowSurface& rhs) = delete;
		WindowSurface& operator=(WindowSurface&& rhs);

		const VulkanInstance& get_instance() const;
		const tz::wsi::window& get_window() const;

		using NativeType = VkSurfaceKHR;
		NativeType native() const;
	private:
		VkSurfaceKHR surface;
		const VulkanInstance* instance;
		const tz::wsi::window* window;
	};

	/**
	 * @ingroup tz_gl_vk
	 * Specifies creation flags for a @ref VulkanInstance.
	 */
	struct VulkanInstanceInfo
	{
		/// Information about the application.
		tz::game_info game_info;
		/// List of extensions to enable. Empty by default.
		InstanceExtensionList extensions = {};
		/// Window from which to create a @ref WindowSurface. If nullptr is passed, no @ref WindowSurface is created. Defaults to nullptr.
		const tz::wsi::window* window = nullptr;
		/// Whether we should attempt to load validation layers or not.
		bool enable_validation_layers = true;
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
		VulkanInstance& operator=(VulkanInstance&& rhs) = default;

		bool has_surface() const;
		bool validation_layers_enabled() const;

		const WindowSurface& get_surface() const;
		const InstanceExtensionList& get_extensions() const;

		using NativeType = VkInstance;
		NativeType native() const;

		bool operator==(const VulkanInstance& rhs) const;

		void ext_begin_debug_utils_label(VkCommandBuffer cmdbuf_native, VkDebugUtilsLabelEXT label) const;
		void ext_end_debug_utils_label(VkCommandBuffer cmdbuf_native) const;
		VkResult ext_set_debug_utils_object_name(VkDevice device_native, VkDebugUtilsObjectNameInfoEXT info) const;
	private:
		void load_extension_functions();

		VulkanInstanceInfo info;
		VkInstance instance;
		std::optional<VulkanDebugMessenger> maybe_debug_messenger;
		std::optional<WindowSurface> maybe_window_surface;
		PFN_vkCmdBeginDebugUtilsLabelEXT ext_vkcmdbegindebugutilslabel = nullptr;
		PFN_vkCmdEndDebugUtilsLabelEXT ext_vkcmdenddebugutilslabel = nullptr;
		PFN_vkSetDebugUtilsObjectNameEXT ext_vksetdebugutilsobjectname = nullptr;
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
