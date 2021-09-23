#ifndef TOPAZ_GL_VK_IMAGE_HPP
#define TOPAZ_GL_VK_IMAGE_HPP
#if TZ_VULKAN
#include "gl/impl/backend/vk/logical_device.hpp"
#include "core/containers/enum_field.hpp"

namespace tz::gl::vk
{
    class CommandBufferRecording;

    class Image
    {
    public:

        enum class Format
        {
            Undefined = VK_FORMAT_UNDEFINED,
            Rgba32Signed = VK_FORMAT_R8G8B8A8_SINT,
            Rgba32SignedNorm = VK_FORMAT_R8G8B8A8_UNORM,
            Rgba32Unsigned = VK_FORMAT_R8G8B8A8_UINT,
            Rgba32UnsignedNorm = VK_FORMAT_R8G8B8A8_UNORM,
            Rgba32sRGB = VK_FORMAT_R8G8B8A8_SRGB,
            Bgra32sRGB = VK_FORMAT_B8G8R8A8_SRGB,
            DepthFloat32 = VK_FORMAT_D32_SFLOAT,
            Bgra32UnsignedNorm = VK_FORMAT_B8G8R8A8_UNORM
        };

        enum class Layout
        {
            Undefined = VK_IMAGE_LAYOUT_UNDEFINED,
            ColourAttachment = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            DepthAttachment = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            Present = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            TransferSource = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            TransferDestination = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            ShaderResource = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };

        enum class Usage
        {
            TransferSource = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,   
            TransferDestination = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            Sampleable = VK_IMAGE_USAGE_SAMPLED_BIT,
            ColourAttachment = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            DepthStencilAttachment = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        };

        using UsageField = tz::EnumField<Usage>;

        Image(const LogicalDevice& device, std::uint32_t width, std::uint32_t height, Format format, UsageField usage, hardware::MemoryResidency residency);
        Image(const Image& copy) = delete;
        Image(Image&& move);
        ~Image();

        Image& operator=(const Image& rhs) = delete;
        Image& operator=(Image&& rhs);

        std::uint32_t get_width() const;
        std::uint32_t get_height() const;
        const LogicalDevice& get_device() const;

        Format get_format() const;
        Usage get_usage() const;
        Layout get_layout() const;
        void set_layout(Image::Layout new_layout) const;
        hardware::MemoryResidency get_memory_residency() const;

        VkImage native() const;
    private:
        VkImage image;
        VmaAllocation alloc;
        const LogicalDevice* device;
        std::uint32_t width;
        std::uint32_t height;
        Usage usage;
        Format format;
        mutable Layout layout;
        hardware::MemoryResidency residency;
    };
}

#endif
#endif // TOPAZ_GL_VK_IMAGE_HPP