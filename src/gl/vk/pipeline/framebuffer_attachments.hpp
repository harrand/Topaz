#ifndef TOPAZ_GL_VK_PIPELINE_FRAMEBUFFER_ATTACHMENTS_HPP
#define TOPAZ_GL_VK_PIPELINE_FRAMEBUFFER_ATTACHMENTS_HPP
#if TZ_VULKAN
#include "gl/vk/setup/swapchain.hpp"
#include "core/containers/basic_list.hpp"

namespace tz::gl::vk::pipeline
{
    enum class AttachmentType
    {
        Colour
    };

    class FramebufferAttachment
    {
    public:
        FramebufferAttachment(const Swapchain& swapchain, AttachmentType type);
        AttachmentType get_type() const;
        VkAttachmentDescription native_description() const;
    private:
        AttachmentType type;
        VkAttachmentDescription description;
    };

    class FramebufferAttachmentList : public tz::BasicList<FramebufferAttachment>
    {
    public:
        FramebufferAttachmentList(std::initializer_list<FramebufferAttachment> attachments = {});
        void add(const FramebufferAttachment& attachment);
        VkSubpassDescription describe_subpass() const;
    private:
        VkAttachmentReference native_reference(std::size_t index) const;
        tz::BasicList<VkAttachmentReference> native_references;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_PIPELINE_FRAMEBUFFER_ATTACHMENTS_HPP