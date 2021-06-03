#if TZ_VULKAN
#include "gl/vk/render_pass.hpp"
#include "gl/vk/framebuffer.hpp"
#include "core/assert.hpp"

namespace tz::gl::vk
{
    RenderPassBuilder::RenderPassBuilder():
    subpasses(){}

    std::vector<const Attachment*> RenderPassBuilder::get_attachments() const
    {
        std::vector<const Attachment*> all_attachments;
        for(const RenderSubpass& subpass : this->subpasses)
        {
            for(const Attachment& attachment : subpass.get_attachments())
            {
                all_attachments.push_back(&attachment);
            }
        }
        return all_attachments;
    }

    std::span<const RenderSubpass> RenderPassBuilder::get_subpasses() const
    {
        return this->subpasses;
    }

    std::span<RenderSubpass> RenderPassBuilder::get_subpasses()
    {
        return this->subpasses;
    }

    bool RenderPassBuilder::has_depth_attachment() const
    {
        auto subpass_attachments = this->get_attachments();
        auto iter = std::find_if(subpass_attachments.begin(), subpass_attachments.end(), [](const Attachment* att)
        {
            return att->get_format() == Image::Format::DepthFloat32;
        });
        return iter != subpass_attachments.end();
    }

    void RenderPassBuilder::remove_subpass(SubpassID subpass)
    {
        this->subpasses.erase(this->subpasses.begin() + subpass);
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
    attachments_offset(parent.get_attachments().size())
    {
    }

    RenderSubpass::Description RenderSubpass::describe() const
    {
        RenderSubpass::Description desc;

        for(const Attachment* attachment : this->parent->get_attachments())
        {
            VkAttachmentReference ref{};
            ref.layout = static_cast<VkImageLayout>(attachment->get_final_image_layout());
            ref.attachment = this->get_attachment_id(*attachment);

            // TODO: Input, Preserve and Resolve attachments?
            switch(attachment->get_final_image_layout())
            {
                case Image::Layout::Present:
                    [[fallthrough]];
                case Image::Layout::TransferSource: // note: for offscreen images
                    ref.layout = static_cast<VkImageLayout>(Image::Layout::ColourAttachment);
                    [[fallthrough]];
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

    std::span<const Attachment> RenderSubpass::get_attachments() const
    {
        return this->attachments;
    }

    std::span<Attachment> RenderSubpass::get_attachments()
    {
        return this->attachments;
    }

    std::size_t RenderSubpass::get_attachment_id(const Attachment& attachment) const
    {
        auto iter = std::find(this->attachments.begin(), this->attachments.end(), attachment);
        if(iter == this->attachments.end())
        {
            // This is not our attachment
            auto parent_list = this->parent->get_attachments();
            auto parent_iter = std::find_if(parent_list.begin(), parent_list.end(), [&attachment](const Attachment* attachment_ptr){return attachment == *attachment_ptr;});
            tz_assert(parent_iter != parent_list.end(), "Attachment not found within subpass nor the parent renderpass.");
            auto global_index = std::distance(parent_list.begin(), parent_iter);
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
        create.dependencyCount = 1;
        VkSubpassDependency initial_dep{};
        initial_dep.srcSubpass = VK_SUBPASS_EXTERNAL;
        initial_dep.dstSubpass = 0;
        initial_dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        initial_dep.srcAccessMask = 0;
        initial_dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        initial_dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        if(builder.has_depth_attachment())
        {
            initial_dep.srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            initial_dep.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            initial_dep.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        }

        create.pDependencies = &initial_dep;

        auto res = vkCreateRenderPass(this->device->native(), &create, nullptr, &this->render_pass);
        tz_assert(res == VK_SUCCESS, "Failed to create render pass");
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

    const LogicalDevice& RenderPass::get_device() const
    {
        return *this->device;
    }

    VkRenderPass RenderPass::native() const
    {
        return this->render_pass;
    }

    RenderPassRun::RenderPassRun(const CommandBuffer& command_buffer, const RenderPass& render_pass, const Framebuffer& framebuffer, VkRect2D render_area, VkClearValue clear_colour):
    command_buffer(&command_buffer)
    {
        VkRenderPassBeginInfo begin{};
        begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        begin.renderPass = render_pass.native();
        begin.framebuffer = framebuffer.native();

        VkClearValue depth_clear_colour{};
        depth_clear_colour.depthStencil = {1.0f, 0};
        std::array<VkClearValue, 2> clear_vals{clear_colour, depth_clear_colour};

        begin.renderArea = render_area;
        begin.clearValueCount = clear_vals.size();
        begin.pClearValues = clear_vals.data();
        
        vkCmdBeginRenderPass(this->command_buffer->native(), &begin, VK_SUBPASS_CONTENTS_INLINE);
    }

    RenderPassRun::~RenderPassRun()
    {
        vkCmdEndRenderPass(this->command_buffer->native());
    }

}

#endif // TZ_VULKAN