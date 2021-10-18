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
	void initialise(tz::GameInfo game_info, tz::ApplicationType app_type);
	void initialise_headless(tz::GameInfo game_info, tz::ApplicationType app_type);
	void terminate();
	/// This is the fixed Vulkan version we will use when building the  engine.
	constexpr tz::Version vulkan_version{1, 2, 0};
	using VkExtension = const char*;

	/**
	 * Only the vulkan extensions listed here are supported. We won't be arbitrarily chucking in random strings into an ExtensionList.
	 */
	enum class Extension
	{
		Swapchain,

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

		constexpr std::array<VkExtension, static_cast<int>(Extension::Count)> extension_names{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

		constexpr VkExtension to_vk_extension(Extension extension)
		{
			using SizeType = decltype(extension_names)::size_type;
			return extension_names[static_cast<SizeType>(static_cast<int>(extension))];
		}
	}


	using VkExtensionList = tz::BasicList<VkExtension>;
	using ExtensionList = tz::BasicList<Extension>;

	/**
	 * Contains information about a Vulkan instance.
	 */
	class VulkanInfo
	{
	public:
		VulkanInfo(tz::GameInfo game_info, ExtensionList extensions = {});

		VkApplicationInfo native() const;
		const ExtensionList& get_extensions() const;
		bool operator==(const VulkanInfo& rhs) const = default;
	private:
		tz::GameInfo game_info;
		std::string engine_name;
		ExtensionList extensions;
	};

	class VulkanInstance
	{
	public:
		VulkanInstance(VulkanInfo info, tz::ApplicationType app_type);
		const VulkanInfo& get_info() const;
		VkInstance native() const;
	private:
		VulkanInfo info;
		tz::ApplicationType app_type;
		VkApplicationInfo info_native;
		VkExtensionList extensions;
		VkInstanceCreateInfo inst_info;
		VkInstance instance;
	};

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
