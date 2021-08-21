#include "gl/impl/frontend/common/render_pass_info.hpp"
#include "gl/api/renderer.hpp"
#include "gl/api/device.hpp"
#include "gl/output.hpp"
#include "gl/resource.hpp"
#include "core/tz.hpp"

namespace tz::gl
{

    tz::gl::TextureFormat RenderPassBuilder::get_output_format() const
    {
        if(this->info.subpasses.colour_attachments.empty())
        {
            return tz::gl::TextureFormat::Null;
        }
        return this->info.subpasses.colour_attachments.front();
    }

    void RenderPassBuilder::add_subpass(RenderSubpassInfo info)
    {
        // TODO: Support for multiple subpasses
        this->info.subpasses = info;
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
       return !this->info.subpasses.colour_attachments.empty() && this->get_output_format() != tz::gl::TextureFormat::Null;
    }

    namespace detail
    {
        RenderPassInfo describe_renderer(const IRendererBuilder& renderer_builder, const IDevice& creator_device)
        {
            RenderPassBuilder builder;
            tz::gl::TextureFormat output_format = tz::gl::TextureFormat::Null;
            if(renderer_builder.get_output()->get_type() == tz::gl::RendererOutputType::Window)
            {
                // Query the device for the window texture format
                output_format = creator_device.get_window_format();
            }
            else
            {
                const auto& renderer_output = *static_cast<const TextureOutput*>(renderer_builder.get_output());
                const IResource* tex_resource = renderer_output.get_first_colour_component()->get_resource();
                tz::gl::TextureFormat output_texture_attachment_format = static_cast<const TextureResource*>(tex_resource)->get_format();
                
                output_format = output_texture_attachment_format;
            }
            RenderSubpassInfo subpass;
            switch(renderer_builder.get_pass())
            {
                case RenderPassAttachment::Colour:
                    subpass.colour_attachments = {output_format};
                    subpass.depth_attachment = std::nullopt;
                break;
                case RenderPassAttachment::Depth:
                    // For now, hardcode to DepthFloat32.
                    // TODO: With output depth textures, this should be customiseable
                    subpass.colour_attachments = {};
                    subpass.depth_attachment = tz::gl::TextureFormat::DepthFloat32;
                break;
                case RenderPassAttachment::ColourDepth:
                    // For now, hardcode to DepthFloat32.
                    // TODO: With output depth textures, this should be customiseable
                    subpass.colour_attachments = {output_format};
                    subpass.depth_attachment = tz::gl::TextureFormat::DepthFloat32;
                break;
                default:
                    tz_error("Unknown RenderPassAttachment");
                break;
            }
            builder.add_subpass(subpass);
            tz_assert(builder.valid(), "When describing a RenderPass via a renderer we did not get a valid result");
            return builder.get_info();
        }
    }
}