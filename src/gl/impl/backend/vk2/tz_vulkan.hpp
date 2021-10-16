#if TZ_VULKAN
#include "core/version.hpp"
#include "core/game_info.hpp"
#include "core/containers/basic_list.hpp"
#include "vulkan/vulkan.h"
#include <cstdint>
namespace tz::gl::vk2
{
	constexpr tz::Version vulkan_version{1, 2, 0};
	namespace util
	{
		using VkVersion = std::uint32_t;
		constexpr VkVersion tz_to_vk_version(tz::Version ver)
		{
			return VK_MAKE_VERSION(ver.major, ver.minor, ver.patch);
		}
	}

	using Extension = const char*;
	using ExtensionList = tz::BasicList<Extension>;

	class VulkanInfo
	{
	public:
		VulkanInfo(tz::GameInfo game_info, ExtensionList extensions = {});

		constexpr VkApplicationInfo native() const
		{
			VkApplicationInfo info{};
			info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			info.pApplicationName = this->game_info.name;
			info.applicationVersion = util::tz_to_vk_version(this->game_info.version);
			
			info.pEngineName = this->engine_name.c_str();
			info.engineVersion = util::tz_to_vk_version(this->game_info.engine.version);
			info.apiVersion = util::tz_to_vk_version(vulkan_version);
			return info;
		}
		const ExtensionList& get_extensions() const;
	private:
		tz::GameInfo game_info;
		std::string engine_name;
		ExtensionList extensions;

	};

	class VulkanInstance
	{
	public:
		VulkanInstance(VulkanInfo info);
	private:
		VulkanInfo info;
		VkApplicationInfo info_native;
		VkInstanceCreateInfo inst_info;
	};
}
#endif // TZ_VULKAN
