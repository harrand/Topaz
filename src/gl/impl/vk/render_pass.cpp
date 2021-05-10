#if TZ_VULKAN
#include "gl/impl/vk/render_pass.hpp"
#include <algorithm>

namespace tz::gl
{
    constexpr vk::Image::Format colour_attachment_format = vk::Image::Format::Rgba32sRGB;

    vk::Attachment default_presentable_colour_attachment()
    {
        return
        {
            colour_attachment_format,
            vk::Attachment::LoadOperation::Clear,
            vk::Attachment::StoreOperation::Store,
            vk::Image::Layout::Undefined,
            vk::Image::Layout::Present
        };
    }

    vk::Attachment default_colour_attachment()
    {
        return
        {
            colour_attachment_format,
            vk::Attachment::LoadOperation::Clear,
            vk::Attachment::StoreOperation::Store,
            vk::Image::Layout::Undefined,
            vk::Image::Layout::ColourAttachment
        };
    }

    vk::Attachment default_depth_attachment()
    {
        return
        {
            vk::Image::Format::DepthFloat32,
            vk::Attachment::LoadOperation::Clear,
            vk::Attachment::StoreOperation::DontCare,
            vk::Attachment::LoadOperation::DontCare,
            vk::Attachment::StoreOperation::DontCare,
            vk::Image::Layout::Undefined,
            vk::Image::Layout::DepthAttachment
        };
    }

    void RenderPassBuilderVulkan::add_pass(RenderPassAttachment attachment)
    {
        switch(attachment)
        {
            case RenderPassAttachment::Colour:
                this->builder.with(vk::Attachments{default_colour_attachment()});
            break;
            case RenderPassAttachment::Depth:
                this->builder.with(vk::Attachments{default_depth_attachment()});
            break;
            case RenderPassAttachment::ColourDepth:
                this->builder.with(vk::Attachments{default_colour_attachment(), default_depth_attachment()});
            break;
        }
    }

    void RenderPassBuilderVulkan::finalise()
    {
        // Find the last subpass with a colour attachment. It should be made into a presentable colour attachment.
        auto get_last_colour_attachment = [](std::span<vk::RenderSubpass> subpasses) -> vk::Attachment*
        {
            for(auto iter = subpasses.rbegin(); iter != subpasses.rend(); iter++)
            {
                for(vk::Attachment& attachment : iter->get_attachments())
                {
                    if(attachment.get_format() == colour_attachment_format)
                    {
                        return &attachment;
                    }
                }
            }
            return nullptr;
        };

        vk::Attachment* last_colour_attachment = get_last_colour_attachment(this->builder.get_subpasses());
        if(last_colour_attachment != nullptr)
        {
            last_colour_attachment->set_final_image_layout(vk::Image::Layout::Present);
        }
    }

    RenderPassVulkan::RenderPassVulkan(const vk::LogicalDevice& device, RenderPassBuilderVulkan builder):
    render_pass(device, builder.builder)
    {
        
    }
}

#endif // TZ_VULKAN