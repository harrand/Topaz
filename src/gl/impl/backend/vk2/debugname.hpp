#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_DEBUGNAME_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_DEBUGNAME_HPP
#if TZ_VULKAN
#include "gl/impl/backend/vk2/logical_device.hpp"

namespace tz::gl::vk2
{
	template<VkObjectType T>
	class DebugNameable
	{
	public:
		DebugNameable(const vk2::LogicalDevice& device, std::uint64_t handle = 0);
		DebugNameable();

		std::string debug_get_name() const;
		void debug_set_name(std::string debug_name);
	protected:
		void debug_set_handle(std::uint64_t handle);
		void debugname_swap(DebugNameable<T>& rhs);
	private:

		const vk2::LogicalDevice* device;
		std::uint64_t handle;
		std::string debug_name = "";
	};
}
#include "gl/impl/backend/vk2/debugname.inl"

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_DEBUGNAME_HPP
