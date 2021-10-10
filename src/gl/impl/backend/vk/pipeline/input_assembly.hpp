#ifndef TOPAZ_GL_VK_PIPELINE_INPUT_ASSEMBLY_HPP
#define TOPAZ_GL_VK_PIPELINE_INPUT_ASSEMBLY_HPP
#if TZ_VULKAN
#include "vulkan/vulkan.h"

namespace tz::gl::vk::pipeline
{
	enum class PrimitiveTopology
	{
		Points = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
		Lines = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
		LineStrips = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
		Triangles = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		TriangleStrips = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP
	};

	class InputAssembly
	{
	public:
		InputAssembly(PrimitiveTopology topology, bool breakable_strips = false);
		VkPipelineInputAssemblyStateCreateInfo native() const;
	private:
		VkPipelineInputAssemblyStateCreateInfo create;
	};
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_PIPELINE_INPUT_ASSEMBLY_HPP