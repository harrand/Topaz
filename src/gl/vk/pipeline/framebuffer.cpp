#if TZ_VULKAN
#include "gl/vk/pipeline/framebuffer.hpp"
#include "core/assert.hpp"

namespace tz::gl::vk::pipeline
{
    FramebufferAttachment::FramebufferAttachment(const Swapchain& swapchain, AttachmentType type):
    type(type),
    description()
    {
        this->description.format = swapchain.native_format();
        this->description.samples = VK_SAMPLE_COUNT_1_BIT;
        // If we're creating an attachment for the swapchain, its format must match that of the swapchain
        // What do we do with the data in the attachment before and after rendering?
        // TODO: Customiseable?
        this->description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        // TODO: Customiseable?
        this->description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        // TODO: Once stencil buffer support arrives this will need to be customiseable.
        this->description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        this->description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        // TODO: Once render-to-texture support arrived, this will need to be customiseable.
        this->description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        this->description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }

    AttachmentType FramebufferAttachment::get_type() const
    {
        return this->type;
    }

    VkAttachmentDescription FramebufferAttachment::native_description() const
    {
        return this->description;
    }

    FramebufferAttachmentList::FramebufferAttachmentList(std::initializer_list<FramebufferAttachment> attachments):
    tz::BasicList<FramebufferAttachment>()
    {
        for(auto attachment : attachments)
        {
            this->add(attachment);
        }
    }

    void FramebufferAttachmentList::add(const FramebufferAttachment& attachment)
    {
        tz::BasicList<FramebufferAttachment>::add(attachment);
        this->native_references.add(this->native_reference(this->length() - 1));
    }

    VkSubpassDescription FramebufferAttachmentList::describe_subpass() const
    {
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        // TODO: Investigate compute subpasses in the future?
        subpass.colorAttachmentCount = this->length();
        subpass.pColorAttachments = this->native_references.data();
        return subpass;
    }

    VkAttachmentReference FramebufferAttachmentList::native_reference(std::size_t index) const
    {
        VkAttachmentReference attach_ref{};
        attach_ref.attachment = index;
        switch((*this)[index].get_type())
        {
            case AttachmentType::Colour:
                attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            break;
            default:
                tz_error("tz::gl::vk::pipeline::FramebufferAttachmentList::native_reference(%zu): Could not recognise attachment type. Did you forget to add support for this attachment?", index);
            break;
        }
        return attach_ref;
    }

}

#endif // TZ_VULKAN