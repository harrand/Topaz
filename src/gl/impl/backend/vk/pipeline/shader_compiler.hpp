#ifndef TOPAZ_GL_VK_PIPELINE_SHADER_COMPILER_HPP
#define TOPAZ_GL_VK_PIPELINE_SHADER_COMPILER_HPP
#if TZ_VULKAN
#include <vector>
#include <optional>
#include <string>

namespace tz::gl::vk
{
	std::optional<std::vector<char>> read_external_shader(std::string shader_name);
}

#endif
#endif // TOPAZ_GL_VK_PIPELINE_SHADER_COMPILER_HPP