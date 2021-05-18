#ifndef TOPAZ_GL_VK_ATTACHMENT_HPP
#define TOPAZ_GL_VK_ATTACHMENT_HPP
#if TZ_VULKAN
#include "gl/vk/image.hpp"

namespace tz::gl::vk
{
    class Attachment
    {
    public:

        enum class StoreOperation
        {
            Store = VK_ATTACHMENT_STORE_OP_STORE, // Store the results
            DontCare = VK_ATTACHMENT_STORE_OP_DONT_CARE // Results won't show-up outside of subpass
        };

        enum class LoadOperation
        {
            Load = VK_ATTACHMENT_LOAD_OP_LOAD, // Preserve any contents from before
            Clear = VK_ATTACHMENT_LOAD_OP_CLEAR, // Clear to a known good value
            DontCare = VK_ATTACHMENT_LOAD_OP_DONT_CARE // Some fast clear technique
        };

        Attachment
        (
            Image::Format format,
            LoadOperation colour_depth_load,
            StoreOperation colour_depth_store,
            Image::Layout initial_layout,
            Image::Layout final_layout
        );

        Attachment
        (
            Image::Format format,
            LoadOperation colour_depth_load,
            StoreOperation colour_depth_store,
            LoadOperation stencil_load,
            StoreOperation stencil_store,
            Image::Layout initial_layout,
            Image::Layout final_layout
        );

        // Format
        Image::Format get_format() const;
        void set_format(Image::Format format);

        // Operations (Colour + Depth)
        LoadOperation get_colour_depth_load_operation() const;
        void set_colour_depth_load_operation(LoadOperation op);
        StoreOperation get_colour_depth_store_operation() const;
        void set_colour_depth_store_operation(StoreOperation op);

        // Operations (Stencil)
        LoadOperation get_stencil_load_operation() const;
        void set_stencil_load_operation(LoadOperation op);
        StoreOperation get_stencil_store_operation() const;
        void set_stencil_store_operation(StoreOperation op);

        // Image Layouts
        Image::Layout get_initial_image_layout() const;
        void set_initial_image_layout(Image::Layout layout);
        Image::Layout get_final_image_layout() const;
        void set_final_image_layout(Image::Layout layout);

        const VkAttachmentDescription& get_description() const;
        bool operator==(const Attachment& rhs) const;
    private:
        VkAttachmentDescription desc;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_ATTACHMENT_HPP