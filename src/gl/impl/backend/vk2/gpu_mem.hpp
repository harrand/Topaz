#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_GPU_MEM_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_GPU_MEM_HPP
#if TZ_VULKAN

namespace tz::gl::vk2
{
	enum class MemoryResidency
	{
		GPU,
		CPU,
		CPUPersistent
	};
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_GPU_MEM_HPP
