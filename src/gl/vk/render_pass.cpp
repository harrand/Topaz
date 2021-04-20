#if TZ_VULKAN
#include "gl/vk/render_pass.hpp"
#include "core/assert.hpp"

namespace tz::gl::vk
{
    RenderPassBuilder::RenderPassBuilder():
    subpass_attachments(),
    subpasses(){}

    std::span<const Attachment* const> RenderPassBuilder::get_attachments() const
    {
        return this->subpass_attachments;
    }

    std::span<const RenderSubpass> RenderPassBuilder::get_subpasses() const
    {
        return this->subpasses;
    }

    RenderSubpass::Description::Description(const RenderSubpass::Description& copy):
    referenced_input_attachments(copy.referenced_input_attachments),
    referenced_colour_attachments(copy.referenced_colour_attachments),
    referenced_depth_stencil_attachments(copy.referenced_depth_stencil_attachments),
    referenced_preserved_attachments(copy.referenced_preserved_attachments)
    {
        this->update_description();
    }

    RenderSubpass::Description::Description(RenderSubpass::Description&& move):
    referenced_input_attachments(std::move(move.referenced_input_attachments)),
    referenced_colour_attachments(std::move(move.referenced_colour_attachments)),
    referenced_depth_stencil_attachments(std::move(move.referenced_depth_stencil_attachments)),
    referenced_preserved_attachments(std::move(move.referenced_preserved_attachments))
    {
        this->update_description();
    }

    void RenderSubpass::Description::update_description()
    {
        this->vk.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        
        this->vk.inputAttachmentCount = this->referenced_input_attachments.size();
        this->vk.pInputAttachments = this->referenced_input_attachments.data();

        this->vk.colorAttachmentCount = this->referenced_colour_attachments.size();
        this->vk.pColorAttachments = this->referenced_colour_attachments.data();

        this->vk.pResolveAttachments = nullptr;

        if(this->referenced_depth_stencil_attachments.has_value())
        {
            this->vk.pDepthStencilAttachment = &this->referenced_depth_stencil_attachments.value();
        }
        else
        {
            this->vk.pDepthStencilAttachment = nullptr;
        }

        this->vk.preserveAttachmentCount = this->referenced_preserved_attachments.size();
        this->vk.pPreserveAttachments = this->referenced_preserved_attachments.data();
        
    }

    RenderSubpass::RenderSubpass(RenderPassBuilder& parent, Attachments attachments):
    parent(&parent),
    attachments(attachments),
    attachments_offset(parent.subpass_attachments.size())
    {
        for(const Attachment& attachment : this->attachments)
        {
            this->parent->subpass_attachments.push_back(&attachment);
        }
    }

    RenderSubpass::Description RenderSubpass::describe() const
    {
        RenderSubpass::Description desc;

        for(const Attachment* attachment : this->parent->subpass_attachments)
        {
            VkAttachmentReference ref{};
            ref.layout = static_cast<VkImageLayout>(attachment->get_final_image_layout());
            ref.attachment = this->get_attachment_id(*attachment);

            // TODO: Input, Preserve and Resolve attachments?
            switch(attachment->get_final_image_layout())
            {
                case Image::Layout::ColourAttachment:
                    desc.referenced_colour_attachments.push_back(ref);
                break;
                case Image::Layout::DepthAttachment:
                    tz_assert(!desc.referenced_depth_stencil_attachments.has_value(), "Can't have more than 1 depth attachment (possibly stencil-related? stencil is NYI)");
                    desc.referenced_depth_stencil_attachments = ref;
                break;
                default:
                    tz_error("Attachment type not implemented");
                break;
            }
        }
        desc.update_description();
        return desc;
    }

    std::size_t RenderSubpass::get_attachment_id(const Attachment& attachment) const
    {
        auto iter = std::find(this->attachments.begin(), this->attachments.end(), attachment);
        if(iter == this->attachments.end())
        {
            // This is not our attachment
            const auto& parent_list = this->parent->subpass_attachments;
            auto iter = std::find_if(parent_list.begin(), parent_list.end(), [&attachment](const Attachment* attachment_ptr){return attachment == *attachment_ptr;});
            tz_assert(iter != parent_list.end(), "Attachment not found within subpass nor the parent renderpass.");
            auto global_index = std::distance(parent_list.begin(), iter);
            return global_index;
        }
        // This is our attachment, we can pretty easily find it.
        auto local_index = std::distance(this->attachments.begin(), iter);
        return this->attachments_offset + local_index;
    }

    RenderPass::RenderPass(const LogicalDevice& device, RenderPassBuilder builder):
    render_pass(VK_NULL_HANDLE),
    device(&device)
    {
        std::vector<VkAttachmentDescription> attachments;
        
        attachments.reserve(builder.get_attachments().size());
        for(const Attachment* attachment_ptr : builder.get_attachments())
        {
            attachments.push_back(attachment_ptr->get_description());
        }

        // Need all descriptions resident in RAM if we're going to reference them all.
        std::vector<RenderSubpass::Description> subpass_descriptions;
        subpass_descriptions.reserve(builder.get_subpasses().size());
        for(const RenderSubpass& subpass : builder.get_subpasses())
        {
            subpass_descriptions.push_back(subpass.describe());
        }
        std::vector<VkSubpassDescription> subpass_vk_descriptions;
        subpass_vk_descriptions.reserve(builder.get_subpasses().size());
        for(const RenderSubpass::Description& desc : subpass_descriptions)
        {
            subpass_vk_descriptions.push_back(desc.vk);
        }
        
        VkRenderPassCreateInfo create{};
        create.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        create.attachmentCount = builder.get_attachments().size();
        create.pAttachments = attachments.data();
        create.subpassCount = builder.get_subpasses().size();
        create.pSubpasses = subpass_vk_descriptions.data();

        auto res = vkCreateRenderPass(this->device->native(), &create, nullptr, &this->render_pass);
    }

    RenderPass::RenderPass(RenderPass&& move):
    render_pass(VK_NULL_HANDLE),
    device(nullptr)
    {
        *this = std::move(move);
    }

    RenderPass::~RenderPass()
    {
        if(this->render_pass != VK_NULL_HANDLE)
        {
            vkDestroyRenderPass(this->device->native(), this->render_pass, nullptr);
        }
    }

    RenderPass& RenderPass::operator=(RenderPass&& rhs)
    {
        std::swap(this->render_pass, rhs.render_pass);
        std::swap(this->device, rhs.device);
        return *this;
    }
}

#endif // TZ_VULKAN