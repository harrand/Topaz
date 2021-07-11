#if TZ_VULKAN
#include "gl/impl/backend/vk/attachment.hpp"

namespace tz::gl::vk
{
    Attachment::Attachment
    (
        Image::Format format,
        LoadOperation colour_depth_load,
        StoreOperation colour_depth_store,
        Image::Layout initial_layout,
        Image::Layout final_layout
    ):
    Attachment
    (
        format,
        colour_depth_load,
        colour_depth_store,
        LoadOperation::DontCare,
        StoreOperation::DontCare,
        initial_layout,
        final_layout
    ){}

    Attachment::Attachment
    (
        Image::Format format,
        LoadOperation colour_depth_load,
        StoreOperation colour_depth_store,
        LoadOperation stencil_load,
        StoreOperation stencil_store,
        Image::Layout initial_layout,
        Image::Layout final_layout
    ):
    desc{}
    {
        this->set_format(format);
        this->set_colour_depth_load_operation(colour_depth_load);
        this->set_colour_depth_store_operation(colour_depth_store);
        this->set_stencil_load_operation(stencil_load);
        this->set_stencil_store_operation(stencil_store);
        this->set_initial_image_layout(initial_layout);
        this->set_final_image_layout(final_layout);
        this->desc.samples = VK_SAMPLE_COUNT_1_BIT;
    }

    // Format
    Image::Format Attachment::get_format() const
    {
        return static_cast<Image::Format>(this->desc.format);
    }

    void Attachment::set_format(Image::Format format)
    {
        this->desc.format = static_cast<VkFormat>(format);
    }

    // Colour & Depth
    Attachment::LoadOperation Attachment::get_colour_depth_load_operation() const
    {
        return static_cast<Attachment::LoadOperation>(this->desc.loadOp);
    }

    void Attachment::set_colour_depth_load_operation(Attachment::LoadOperation op)
    {
        this->desc.loadOp = static_cast<VkAttachmentLoadOp>(op);
    }

    Attachment::StoreOperation Attachment::get_colour_depth_store_operation() const
    {
        return static_cast<Attachment::StoreOperation>(this->desc.storeOp);
    }

    void Attachment::set_colour_depth_store_operation(Attachment::StoreOperation op)
    {
        this->desc.storeOp = static_cast<VkAttachmentStoreOp>(op);
    }

    // Stencil
    Attachment::LoadOperation Attachment::get_stencil_load_operation() const
    {
        return static_cast<Attachment::LoadOperation>(this->desc.stencilLoadOp);
    }

    void Attachment::set_stencil_load_operation(Attachment::LoadOperation op)
    {
        this->desc.stencilLoadOp = static_cast<VkAttachmentLoadOp>(op);
    }

    Attachment::StoreOperation Attachment::get_stencil_store_operation() const
    {
        return static_cast<Attachment::StoreOperation>(this->desc.stencilStoreOp);
    }

    void Attachment::set_stencil_store_operation(Attachment::StoreOperation op)
    {
        this->desc.stencilStoreOp = static_cast<VkAttachmentStoreOp>(op);
    }

    Image::Layout Attachment::get_initial_image_layout() const
    {
        return static_cast<Image::Layout>(this->desc.initialLayout);
    }

    void Attachment::set_initial_image_layout(Image::Layout layout)
    {
        this->desc.initialLayout = static_cast<VkImageLayout>(layout);
    }

    Image::Layout Attachment::get_final_image_layout() const
    {
        return static_cast<Image::Layout>(this->desc.finalLayout);
    }

    void Attachment::set_final_image_layout(Image::Layout layout)
    {
        this->desc.finalLayout = static_cast<VkImageLayout>(layout);
    }

    const VkAttachmentDescription& Attachment::get_description() const
    {
        return this->desc;
    }

    bool Attachment::operator==(const Attachment& rhs) const
    {
        return this->get_format() == rhs.get_format()
            && this->get_colour_depth_load_operation() == rhs.get_colour_depth_load_operation()
            && this->get_colour_depth_store_operation() == rhs.get_colour_depth_store_operation()
            && this->get_stencil_load_operation() == rhs.get_stencil_load_operation()
            && this->get_stencil_store_operation() == rhs.get_stencil_store_operation()
            && this->get_initial_image_layout() == rhs.get_initial_image_layout()
            && this->get_final_image_layout() == rhs.get_final_image_layout();
    }

}

#endif // TZ_VULKAN