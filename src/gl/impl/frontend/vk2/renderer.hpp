#ifndef TOPAZ_GL_IMPL_FRONTEND_VK2_RENDERER_HPP
#define TOPAZ_GL_IMPL_FRONTEND_VK2_RENDERER_HPP
#if TZ_VULKAN
#include "gl/impl/frontend/common/renderer.hpp"
#include "gl/impl/backend/vk2/image.hpp"
#include "gl/impl/backend/vk2/image_view.hpp"
#include "gl/impl/backend/vk2/render_pass.hpp"
#include "gl/impl/backend/vk2/framebuffer.hpp"

namespace tz::gl
{
	using RendererBuilderVulkan2 = RendererBuilderBase;

	struct RendererBuilderDeviceInfoVulkan2
	{
		const vk2::LogicalDevice* device;
		std::span<vk2::Image> output_images;
	};

	namespace detail
	{
		struct IOManagerInfo
		{
			const vk2::LogicalDevice* device;
			std::span<vk2::Image> output_images;
			RendererOutputType output_type;
		};

		class IOManager
		{
		public:
			IOManager(IOManagerInfo info);
			std::span<vk2::Framebuffer> get_output_framebuffers();
		private:
			std::vector<vk2::ImageView> output_views;
			vk2::RenderPass render_pass;
			std::vector<vk2::Framebuffer> output_framebuffers;
		};

		class ResourceManager
		{

		};
	}

	class RendererVulkan2 : public RendererBase
	{
	public:
		RendererVulkan2(RendererBuilderVulkan2 builder, RendererBuilderDeviceInfoVulkan2 device_info);
		virtual void set_clear_colour(tz::Vec4 clear_colour) final{}
		virtual IComponent* get_component(ResourceHandle handle) final{return nullptr;}
		
		virtual void render() final{}
		virtual void render(RendererDrawList draws) final{}
	private:
		detail::IOManager io_manager;
		detail::ResourceManager resource_manager;
	};
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_FRONTEND_VK2_RENDERER_HPP
