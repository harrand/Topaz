#if TZ_VULKAN
#include "gl/impl/backend/vk/hardware/memory.hpp"
#include "gl/impl/backend/vk/hardware/device.hpp"
#include "core/assert.hpp"

namespace tz::gl::vk::hardware
{
	MemoryProperties::MemoryProperties(const Device& device):
	modules()
	{
		VkPhysicalDeviceMemoryProperties props;
		vkGetPhysicalDeviceMemoryProperties(device.native(), &props);

		for(std::uint32_t i = 0; i < props.memoryTypeCount; i++)
		{
			VkMemoryType vk_type = props.memoryTypes[i];
			VkMemoryPropertyFlags flags = vk_type.propertyFlags;
			this->modules.push_back(MemoryModule{&device, MemoryProperties::parse_field(flags), i, vk_type.heapIndex});
		}
	}

	auto MemoryProperties::begin() const
	{
		return this->modules.begin();
	}

	auto MemoryProperties::end() const
	{
		return this->modules.end();
	}

	tz::BasicList<MemoryModule> MemoryProperties::get_modules_matching(MemoryTypeField field) const
	{
		tz::BasicList<MemoryModule> matching_modules;
		for(const MemoryModule& mod : this->modules)
		{
			if(mod.types == field)
			{
				matching_modules.add(mod);
			}
		}
		return matching_modules;
	}

	MemoryModule MemoryProperties::unsafe_get_some_module_matching(MemoryTypeField field) const
	{
		return this->get_modules_matching(field)[0];
	}

	/*static*/ MemoryTypeField MemoryProperties::parse_field(VkMemoryPropertyFlags flags)
	{
		MemoryTypeField field;
		constexpr MemoryType supported_types[] = 
		{
			MemoryType::DeviceLocal,
			MemoryType::HostVisible,
			MemoryType::HostCoherent,
			MemoryType::HostCached
		};

		for(MemoryType t : supported_types)
		{
			if(flags & static_cast<VkMemoryPropertyFlagBits>(t))
			{
				field |= t;
			}
		}
		
		return field;
	}
}

#endif // TZ_VULKAN