#include "core/resource_manager.hpp"
#include <fstream>
#include <sstream>

namespace tz
{
	ResourceManager::ResourceManager(std::string resource_parent_path): path(resource_parent_path)
	{
		if(!this->path.empty())
		{
			if(this->path.back() != '\\' || this->path.back() != '/')
				this->path += '/';
		}
	}

	const std::string& ResourceManager::get_path() const
	{
		return this->path;
	}

	std::vector<std::byte> ResourceManager::load_raw(std::string relative_path) const
	{
		std::vector<std::byte> bytes;
		auto text = this->load_text(relative_path);
		if(text.has_value())
		{
			for(char c : text.value())
				bytes.push_back(static_cast<std::byte>(c));
		}
		return bytes;
	}

	std::optional<std::string> ResourceManager::load_text(std::string relative_path) const
	{
		std::ifstream file{this->path + relative_path};
		if(!file.good())
			return {std::nullopt};
		std::stringstream ss;
		ss << file.rdbuf();
		return {ss.str()};
	}
}