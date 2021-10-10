#ifndef TOPAZ_GL_IMPL_FRONTEND_VK_RENDER_PASS_HPP
#define TOPAZ_GL_IMPL_FRONTEND_VK_RENDER_PASS_HPP
#if TZ_VULKAN
#include "gl/impl/frontend/common/render_pass_info.hpp"
#include "gl/impl/backend/vk/render_pass.hpp"

namespace tz::gl
{
	class RenderPassBuilderVulkan
	{
	public:
		RenderPassBuilderVulkan(RenderPassInfo info, bool presentable_output);
		vk::RenderPassBuilder vk_get() const;
	private:
		RenderPassInfo info;
		bool presentable_output;
	};

	class IDevice;
	class DeviceWindowBufferVulkan;

	struct RenderPassDeviceInfoVulkan
	{
		const IDevice* device;
		const DeviceWindowBufferVulkan* window_buffer;
		const vk::LogicalDevice* vk_device;
	};

	class RenderPassVulkan
	{
	public:
		RenderPassVulkan(RenderPassBuilderVulkan builder, RenderPassDeviceInfoVulkan device_info);
		const vk::RenderPass& vk_get_render_pass() const;
	private:
		vk::RenderPass render_pass;
	};
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_FRONTEND_VK_RENDER_PASS_HPP