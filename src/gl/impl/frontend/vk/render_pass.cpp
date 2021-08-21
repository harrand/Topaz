#if TZ_VULKAN
#include "gl/impl/frontend/vk/render_pass.hpp"
#include "gl/device.hpp"

namespace tz::gl
{
    RenderPassBuilderVulkan::RenderPassBuilderVulkan(RenderPassInfo info, bool presentable_output):
    info(info),
    presentable_output(presentable_output){}

    vk::RenderPassBuilder RenderPassBuilderVulkan::vk_get() const
    {
        vk::RenderPassBuilder builder;
        vk::Attachment colour_attachment
        {
            vk::Image::Format::Undefined, // Placeholder
            vk::Attachment::LoadOperation::Clear,
            vk::Attachment::StoreOperation::Store,
            vk::Image::Layout::Undefined,
            vk::Image::Layout::Undefined // Placeholder
        };
        
        // Amend the colour attachment to match what we need
        if(vk::is_headless())
        {
            colour_attachment.set_final_image_layout(vk::Image::Layout::ColourAttachment);
        }
        else
        {
            if(this->presentable_output)
            {
                // Image will be presented to the screen
                colour_attachment.set_final_image_layout(vk::Image::Layout::Present);
            }
            else
            {
                // Image will be used as a Resource within another renderer.
                colour_attachment.set_final_image_layout(vk::Image::Layout::ShaderResource);
            }
        }

        tz_assert(this->info.subpasses.colour_attachments.length() == 1, "Multiple colour attachments are not yet supported");
        vk::Image::Format colour_format;
        switch(this->info.subpasses.colour_attachments.front())
        {
            case tz::gl::TextureFormat::Rgba32Signed:
                colour_format = vk::Image::Format::Rgba32Signed;
            break;
            case tz::gl::TextureFormat::Rgba32Unsigned:
                colour_format = vk::Image::Format::Rgba32Unsigned;
            break;
            case tz::gl::TextureFormat::Rgba32sRGB:
                colour_format = vk::Image::Format::Rgba32sRGB;
            break;
            case tz::gl::TextureFormat::DepthFloat32:
                colour_format = vk::Image::Format::DepthFloat32;
            break;
            case tz::gl::TextureFormat::Bgra32UnsignedNorm:
                colour_format = vk::Image::Format::Bgra32UnsignedNorm;
            break;
            default:
                tz_error("Unrecognised TextureFormat (Vulkan)");
            break;
        }
        colour_attachment.set_format(colour_format);

        if(this->info.subpasses.depth_attachment != std::nullopt)
        {
            vk::Attachment depth_attachment
            {
                vk::Image::Format::DepthFloat32,
                vk::Attachment::LoadOperation::Clear,
                vk::Attachment::StoreOperation::Store,
                vk::Attachment::LoadOperation::DontCare,
                vk::Attachment::StoreOperation::DontCare,
                vk::Image::Layout::Undefined,
                vk::Image::Layout::DepthAttachment  
            };

            builder.with(vk::Attachments{colour_attachment, depth_attachment});
        }
        else
        {
            builder.with(vk::Attachments{colour_attachment});
        }
        return builder;
    }

    RenderPassVulkan::RenderPassVulkan(RenderPassBuilderVulkan builder, RenderPassDeviceInfoVulkan device_info):
    render_pass(*device_info.vk_device, builder.vk_get()){}

    const vk::RenderPass& RenderPassVulkan::vk_get_render_pass() const
    {
        return this->render_pass;
    }
}
#endif // TZ_VULKAN