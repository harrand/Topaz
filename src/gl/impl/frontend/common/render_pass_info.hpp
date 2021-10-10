#ifndef TOPAZ_GL_IMPL_FRONTEND_COMMON_RENDER_PASS_HPP
#define TOPAZ_GL_IMPL_FRONTEND_COMMON_RENDER_PASS_HPP
#include "core/containers/basic_list.hpp"
#include "gl/declare/texture.hpp"
#include <optional>

namespace tz::gl
{
	// Represents information about the images used within a subpass
	struct RenderSubpassInfo
	{
		// TODO: Input attachments? Right now we don't expect to support more than 1 subpass, so these shouldn't be needed.
		tz::BasicList<tz::gl::TextureFormat> colour_attachments = {};
		std::optional<tz::gl::TextureFormat> depth_attachment = std::nullopt;
	};

	struct RenderPassInfo
	{
		// TODO: Multiple subpasses
		RenderSubpassInfo subpasses = {};
	};

	class RenderPassBuilder
	{
	public:
		RenderPassBuilder() = default;
		tz::gl::TextureFormat get_output_format() const;

		void add_subpass(RenderSubpassInfo info);

		const RenderPassInfo& get_info() const;

		bool valid() const;
	private:
		RenderPassInfo info;
	};


	class IRendererBuilder;
	class IDevice;

	namespace detail
	{
		RenderPassInfo describe_renderer(const IRendererBuilder& renderer_builder, const IDevice& creator_device);
	}
}

#endif // TOPAZ_GL_IMPL_FRONTEND_COMMON_RENDER_PASS_HPP