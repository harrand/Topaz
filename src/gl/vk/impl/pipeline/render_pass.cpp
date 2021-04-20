#if TZ_VULKAN
#include "gl/vk/impl/pipeline/render_pass.hpp"
#include "core/assert.hpp"

namespace tz::gl::vk::pipeline
{
    RenderPass::RenderPass(const LogicalDevice& device, std::initializer_list<FramebufferAttachmentList> subpasses):
    device(&device),
    subpasses(subpasses),
    render_pass(VK_NULL_HANDLE)
    {
        VkRenderPassCreateInfo create{};
        create.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        std::vector<VkAttachmentDescription> all_descriptions;
        std::vector<VkSubpassDescription> subpass_descriptions;
        for(const FramebufferAttachmentList& subpass : this->subpasses)
        {
            for(const FramebufferAttachment& attachment : subpass)
            {
                all_descriptions.push_back(attachment.native_description());
            }
            subpass_descriptions.push_back(subpass.describe_subpass());
        }
        create.attachmentCount = all_descriptions.size();
        create.pAttachments = all_descriptions.data();
        create.subpassCount = this->subpasses.size();
        create.pSubpasses = subpass_descriptions.data();

        auto res = vkCreateRenderPass(this->device->native(), &create, nullptr, &this->render_pass);
        tz_assert(res == VK_SUCCESS, "tz::gl::vk::pipeline::RenderPass::RenderPass(...): Failed to create RenderPass");
    }

    RenderPass::RenderPass(RenderPass& move):
    device(nullptr),
    subpasses(),
    render_pass(VK_NULL_HANDLE)
    {
        *this = std::move(move);
    }

    RenderPass::~RenderPass()
    {
        if(this->render_pass != VK_NULL_HANDLE)
        {
            vkDestroyRenderPass(this->device->native(), this->render_pass, nullptr);
            this->render_pass = VK_NULL_HANDLE;
        }
    }

    RenderPass& RenderPass::operator=(RenderPass&& rhs)
    {
        std::swap(this->device, rhs.device);
        std::swap(this->subpasses, rhs.subpasses);
        std::swap(this->render_pass, rhs.render_pass);
        return *this;
    }

    VkRenderPass RenderPass::native() const
    {
        return this->render_pass;
    }

    const LogicalDevice& RenderPass::get_device() const
    {
        tz_assert(this->device != nullptr, "tz::gl::vk::pipeline::RenderPass::get_device(): Device was null");
        return *this->device;
    }
}


#endif // TZ_VULKAN