#include "core/window.hpp"
#if TZ_VULKAN
#include "core/version.hpp"
#include "core/game_info.hpp"
#include "core/tz.hpp"
#include "gl/impl/backend/vk2/extensions.hpp"
#include <cstdint>
namespace tz::gl::vk2
{
	class VulkanInstance;
	class WindowSurface;

	/**
	 * @ingroup tz_gl
	 * @defgroup tz_gl_vk Vulkan Backend (tz::gl::vk2)
	 * Documentation for those working with the Vulkan Backend.
	 */

	/**
	 * @ingroup tz_gl_vk
	 * @defgroup tz_gl_vk_presentation Presentation and Window Surface Interation (WSI)
	 * Documentation for functionality related to presenting images to existing windows.
	 */

	/**
	 * @ingroup tz_gl_vk
	 * @defgroup tz_gl_vk_extension Extensions and Features
	 * Documentation for functionality related to instance/device extensions, and optional features.
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
	 * @brief Initialise the vulkan backend for headless rendering.
	 * @note This is intended to be invoked automatically by @ref tz::initialise however this is not yet implemented - You should invoke this yourself at the start of your program directly after `tz::initialise`
	 */
	void initialise_headless(tz::GameInfo game_info, tz::ApplicationType app_type);
	/**
	 * @brief Terminate the vulkan backend
	 */
	void terminate();

	/**
	 * @brief Retrieve a reference to the current Vulkan Instance. This will have been created during initialisation.
	 */
	const VulkanInstance& get();
	bool is_headless();
	const WindowSurface& get_window_surface();

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

	/**
	 * @ingroup tz_gl_vk
	 * Specifies parameters for a newly created @ref VulkanInstance.
	 */
	class VulkanInfo
	{
	public:
		/**
		 * @brief Construct a VulkanInfo from an existing @ref tz::GameInfo and an @ref ExtensionList of instance extensions - Note that these are instance extensions, not device extensions.
		 * @param game_info Information about the application being ran.
		 * @param extensions List of instance extensions to enable.
		 */
		VulkanInfo(tz::GameInfo game_info, InstanceExtensionList extensions = {});

		VkApplicationInfo native() const;
		/**
		 * Retrieve a list of instance extensions to be used by a VulkanInstance.
		 * @return List of instance extensions to be enabled by the next VulkanInstance using this struct.
		 */
		const InstanceExtensionList& get_extensions() const;
		/**
		 * Query as to whether a spawned VulkanInstance would support debug validation layer messengers.
		 * Debug layer messengers automatically assert on any Vulkan-API validation layer errors. This is enabled when the Extension::DebugMessenger is specified, and built under TZ_DEBUG.
		 * @return True if vulkan-api errors will tz_error, otherwise false.
		 */
		bool has_debug_validation() const;
		bool operator==(const VulkanInfo& rhs) const = default;
	private:
		tz::GameInfo game_info;
		std::string engine_name;
		InstanceExtensionList extensions;
	};

	class VulkanInstance;

	class VulkanDebugMessenger
	{
	public:
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
	 * @ingroup tz_gl_vk
	 * Represents a Vulkan Instance
	 */
	class VulkanInstance
	{
	public:
		/**
		 * @brief Construct an instance.
		 * You probably don't need to do this; an instance is setup for you automatically during initialisation.
		 * @param info Information about the instance.
		 * @param app_type Application type. Headless applications require slightly modified vulkan instances, for example.
		 */
		VulkanInstance(VulkanInfo info, tz::ApplicationType app_type);
		VulkanInstance(const VulkanInstance& copy) = delete;
		VulkanInstance(VulkanInstance&& move) = delete;
		~VulkanInstance();
		VulkanInstance& operator=(const VulkanInstance& rhs) = delete;
		VulkanInstance& operator=(VulkanInstance&& rhs) = delete;
		/**
		 * @brief Retrieve the @ref VulkanInfo used to construct this instance.
		 */
		const VulkanInfo& get_info() const;
		VkInstance native() const;
		bool operator==(const VulkanInstance& rhs) const;
	private:
		VulkanInfo info;
		tz::ApplicationType app_type;
		VkApplicationInfo info_native;
		util::VkExtensionList extensions;
		VkInstanceCreateInfo inst_info;
		VkInstance instance;
		std::optional<VulkanDebugMessenger> debug_messenger;
	};

	/**
	 * @ingroup tz_gl_vk_presentation
	 * Create a representation for an existing Window surface.
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

		VkSurfaceKHR native() const;
		const VulkanInstance& get_instance() const;
	private:
		VkSurfaceKHR surface;
		const VulkanInstance* instance;
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
