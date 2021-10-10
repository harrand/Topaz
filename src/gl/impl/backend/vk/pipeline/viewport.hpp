#ifndef TOPAZ_GL_VK_PIPELINE_VIEWPORT_HPP
#define TOPAZ_GL_VK_PIPELINE_VIEWPORT_HPP
#if TZ_VULKAN
#include "gl/impl/backend/vk/swapchain.hpp"

namespace tz::gl::vk::pipeline
{
	class ViewportState
	{
	public:
		ViewportState(const Swapchain& swapchain, bool use_opengl_coordinate_system = false);
		ViewportState(const Image& image, bool use_opengl_coordinate_system = false);
		VkPipelineViewportStateCreateInfo native() const;
	private:
		ViewportState(float width, float height, VkExtent2D extent, bool use_opengl_coordinate_system);
		VkPipelineViewportStateCreateInfo create;
		VkViewport viewport;
		VkRect2D scissor;
	};
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_PIPELINE_VIEWPORT_HPP