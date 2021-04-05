#ifndef TOPAZ_CORE_RESOURCE_MANAGER_HPP
#define TOPAZ_CORE_RESOURCE_MANAGER_HPP
#include <string>
#include <vector>
#include <cstddef>
#include <optional>

namespace tz
{
	/**
	 * ResourceManagers target a given path and can read files relative to that path.
	 * The global ResourceManager is available at `tz::res()`. This always targets the library Topaz root directory (where its CMakeLists.txt lives).
	 */
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