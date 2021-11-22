#ifndef TOPAZ_GL_IMPL_BACKEND_Vk2_FRAMEBUFFER_HPP
#define TOPAZ_GL_IMPL_BACKEND_Vk2_FRAMEBUFFER_HPP
#if TZ_VULKAN
#include "gl/impl/backend/vk2/render_pass.hpp"
#include "gl/impl/backend/vk2/image_view.hpp"

namespace tz::gl::vk2
{

	/**
	 * @ingroup tz_gl_vk_graphics_pipeline_render_pass
	 * Specifies creation flags for a @ref Framebuffer.
	 */
	struct FramebufferInfo
	{
		/// Query as to whether this info is valid and can be used to create a @ref Framebuffer.
		bool valid() const;

		/// RenderPass that will target the frame buffer.
		const RenderPass* render_pass;
		/// List of output attachments from the RenderPass.
		tz::BasicList<const ImageView*> attachments;
		/// Dimensions of the frame buffer.
		tz::Vector<std::uint32_t, 2> dimensions;
	};

	/**
	 * @ingroup tz_gl_vk_graphics_pipeline_render_pass
	 * Represents a render target for a @ref RenderPass.
	 */
	class Framebuffer
	{
	public:
		Framebuffer(FramebufferInfo info);
		Framebuffer(const Framebuffer& copy) = delete;
		Framebuffer(Framebuffer&& move);
		~Framebuffer();

		Framebuffer& operator=(const Framebuffer& rhs) = delete;
		Framebuffer& operator=(Framebuffer&& rhs);

		const LogicalDevice* get_device() const;
	private:
		FramebufferInfo info;
		VkFramebuffer framebuffer;
	};
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_Vk2_FRAMEBUFFER_HPP
