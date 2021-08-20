#ifndef TOPAZ_GL_IMPL_FRONTEND_COMMON_RENDER_PASS_HPP
#define TOPAZ_GL_IMPL_FRONTEND_COMMON_RENDER_PASS_HPP
#include "core/containers/basic_list.hpp"
#include "gl/impl/frontend/common/resource.hpp"
#include "gl/impl/frontend/common/render_pass_attachment.hpp"

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
        tz::gl::TextureFormat output_format = tz::gl::TextureFormat::Null;
    };

    class RenderPassBuilder
    {
    public:
        RenderPassBuilder() = default;
        void set_output_format(tz::gl::TextureFormat output_format);
        tz::gl::TextureFormat get_output_format() const;

        const RenderPassInfo& get_info() const;

        bool valid() const;
    private:
        RenderPassInfo info;
    };
}

#endif // TOPAZ_GL_IMPL_FRONTEND_COMMON_RENDER_PASS_HPP