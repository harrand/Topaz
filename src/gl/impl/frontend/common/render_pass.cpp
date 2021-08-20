#include "gl/impl/frontend/common/render_pass.hpp"
#include "core/tz.hpp"

namespace tz::gl
{
    void RenderPassBuilder::set_output_format(tz::gl::TextureFormat output_format)
    {
        this->info.output_format = output_format;
    }

    tz::gl::TextureFormat RenderPassBuilder::get_output_format() const
    {
        return this->info.output_format;
    }

    const RenderPassInfo& RenderPassBuilder::get_info() const
    {
        return this->info;
    }

    bool RenderPassBuilder::valid() const
    {
        /* Valid if:
            - We have a valid output format that isn't TextureFormat::Null
            - The final subpass has at least one colour attachment
        */
       bool valid_output_format = this->info.output_format != tz::gl::TextureFormat::Null;
       bool has_colour_attachment = !this->info.subpasses.colour_attachments.empty();
       return valid_output_format && has_colour_attachment;
    }
}