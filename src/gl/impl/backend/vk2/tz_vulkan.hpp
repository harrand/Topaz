#if TZ_VULKAN
#include "core/version.hpp"
#include "core/game_info.hpp"
#include "core/containers/basic_list.hpp"
#include "core/tz.hpp"
#include "vulkan/vulkan.h"
#include <array>
#include <cstdint>
namespace tz::gl::vk2
{
	/**
	 * @ingroup tz_gl
	 * @defgroup tz_gl_vk Vulkan Backend (tz::gl::vk2)
	 * Documentation for those working with the Vulkan Backend.
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

	class VulkanInstance;

	/**
	 * @brief Retrieve a reference to the current Vulkan Instance. This will have been created during initialisation.
	 */
	const VulkanInstance& get();

	constexpr tz::Version vulkan_version{1, 2, 0};

	using VkExtension = const char*;

	/**
	 * @brief Only the vulkan extensions listed here are supported. We won't be arbitrarily chucking in random strings into an ExtensionList.
	 */
	enum class Extension
	{
		/// - Enables Swapchain operations (required to create a swapchain and present images)
		Swapchain,
		/// - Enables Debug Messenger (only supported on TZ_DEBUG)
		DebugMessenger,
		
		Count	
	};

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

		constexpr std::array<VkExtension, static_cast<int>(Extension::Count)> extension_names{VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME};

		constexpr VkExtension to_vk_extension(Extension extension)
		{
			using SizeType = decltype(extension_names)::size_type;
			return extension_names[static_cast<SizeType>(static_cast<int>(extension))];
		}

		constexpr Extension to_tz_extension(VkExtension extension)
		{
			auto iter = std::find(extension_names.begin(), extension_names.end(), extension);
			if(iter != extension_names.end())
			{
				return static_cast<Extension>(static_cast<int>(std::distance(extension_names.begin(), iter)));
			}
			return Extension::Count;
		}
	}

	using VkExtensionList = tz::BasicList<VkExtension>;
	using ExtensionList = tz::EnumField<Extension>;

	/**
	 * @brief Contains information about a Vulkan instance.
	 */
	class VulkanInfo
	{
	public:
		/**
		 * @brief Construct a VulkanInfo from an existing @ref tz::GameInfo and an @ref ExtensionList of instance extensions - Note that these are instance extensions, not device extensions.
		 * @param game_info Information about the application being ran.
		 * @param extensions List of instance extensions to enable.
		 */
		VulkanInfo(tz::GameInfo game_info, ExtensionList extensions = {});

		VkApplicationInfo native() const;
		/**
		 * Retrieve a list of instance extensions to be used by a VulkanInstance.
		 * @return List of instance extensions to be enabled by the next VulkanInstance using this struct.
		 */
		const ExtensionList& get_extensions() const;
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
		ExtensionList extensions;
	};

	class VulkanInstance;

	class VulkanDebugMessenger
	{
	public:
		VulkanDebugMessenger(const VulkanInstance& instance);
		~VulkanDebugMessenger();
	private:
		VkDebugUtilsMessengerEXT debug_messenger;	
		const VulkanInstance* instance;
	};

	/**
	 * @brief Represents a Vulkan Instance
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
		~VulkanInstance();
		/**
		 * @brief Retrieve the @ref VulkanInfo used to construct this instance.
		 */
		const VulkanInfo& get_info() const;
		VkInstance native() const;
	private:
		VulkanInfo info;
		tz::ApplicationType app_type;
		VkApplicationInfo info_native;
		VkExtensionList extensions;
		VkInstanceCreateInfo inst_info;
		VkInstance instance;
		std::optional<VulkanDebugMessenger> debug_messenger;
	};

	/**
	 * @}
	 */

	class WindowSurface
	{
	public:
		WindowSurface(const VulkanInstance& instance, const tz::Window& window);
		~WindowSurface();
	private:
		VkSurfaceKHR surface;
		const VulkanInstance* instance;
	};
}
#endif // TZ_VULKAN
