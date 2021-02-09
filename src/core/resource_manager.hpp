#ifndef TOPAZ_CORE_RESOURCE_MANAGER_HPP
#define TOPAZ_CORE_RESOURCE_MANAGER_HPP
#include <string>
#include <vector>
#include <cstddef>
#include <optional>

namespace tz
{
	class ResourceManager
	{
	public:
		ResourceManager(std::string resource_parent_path);
		const std::string& get_path() const;
		std::vector<std::byte> load_raw(std::string relative_path) const;
		std::optional<std::string> load_text(std::string relative_path) const;
	private:
		std::string path;
	};
}

#endif // TOPAZ_CORE_RESOURCE_MANAGER_HPP