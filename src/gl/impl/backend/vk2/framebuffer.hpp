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
		tz::BasicList<ImageView*> attachments;
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

		/**
		 * Retrieve the width and height of the framebuffer. All attachments must have the same dimensions.
		 * @return Dimensions of the framebuffer, in pixels.
		 */
		tz::Vec2ui get_dimensions() const;
		/**
		 * Retrieve the @ref RenderPass that this Framebuffer expects to act as a target for.
		 * @return Reference to parent @ref RenderPass.
		 */
		const RenderPass& get_pass() const;
		/**
		 * Retrieve the @ref LogicalDevice that spawned this Framebuffer.
		 * @note This is identical to the @ref LogicalDevice that spawned the @ref RenderPass referenced by `this->get_pass()`.
		 */
		const LogicalDevice& get_device() const;
		/**
		 * Retrieve a list of imageviews corresponding to each attachment.
		 * @return List of ImageViews. The i'th @ref ImageView corresponds to the i'th output attachment from the @ref RenderPass referenced by `this->get_pass()`.
		 */
		tz::BasicList<const ImageView*> get_attachment_views() const;
		/**
		 * Retrieve a list of imageviews corresponding to each attachment.
		 * @return List of ImageViews. The i'th @ref ImageView corresponds to the i'th output attachment from the @ref RenderPass referenced by `this->get_pass()`.
		 */
		tz::BasicList<ImageView*> get_attachment_views();
		void set_render_pass(RenderPass& render_pass);

		using NativeType = VkFramebuffer;
		NativeType native() const;
	private:
		FramebufferInfo info;
		VkFramebuffer framebuffer;
	};
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_Vk2_FRAMEBUFFER_HPP
