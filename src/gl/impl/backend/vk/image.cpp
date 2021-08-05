#if TZ_VULKAN
#include "gl/impl/backend/vk/image.hpp"
#include "gl/impl/backend/vk/command.hpp"

namespace tz::gl::vk
{
    Image::Image(const LogicalDevice& device, std::uint32_t width, std::uint32_t height, Image::Format format, Image::UsageField usage, hardware::MemoryResidency residency):
    image(VK_NULL_HANDLE),
    alloc(),
    device(&device),
    width(width),
    height(height),
    usage(static_cast<Image::Usage>(usage)),
    format(format),
    layout(Image::Layout::Undefined),
    residency(residency)
    {
        VkImageCreateInfo create{};
        create.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        create.imageType = VK_IMAGE_TYPE_2D;
        create.extent.width = width;
        create.extent.height = height;
        create.extent.depth = 1;
        create.mipLevels = 1;
        create.arrayLayers = 1;
        create.format = static_cast<VkFormat>(format);
        create.tiling = VK_IMAGE_TILING_OPTIMAL;
        create.initialLayout = static_cast<VkImageLayout>(this->layout);
        create.usage = static_cast<VkImageUsageFlags>(this->usage);
        create.samples = VK_SAMPLE_COUNT_1_BIT;
        create.flags = 0;

        VmaAllocationCreateInfo alloc_info{};
        switch(this->residency)
        {
            case hardware::MemoryResidency::CPU:
                alloc_info.usage = VMA_MEMORY_USAGE_CPU_ONLY;
            break;
            case hardware::MemoryResidency::GPU:
                alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            break;
            default:
                tz_error("Unexpected MemoryResidency");
            break;
        }

        auto res = vmaCreateImage(this->device->native_allocator(), &create, &alloc_info, &this->image, &this->alloc, nullptr);
        tz_assert(res == VK_SUCCESS, "Failed to create image");
    }

    Image::Image(Image&& move):
    image(VK_NULL_HANDLE),
    alloc(),
    device(nullptr),
    width(0),
    height(0),
    usage(),
    format(Image::Format::Undefined),
    layout(Image::Layout::Undefined),
    residency()
    {
        *this = std::move(move);
    }

    Image::~Image()
    {
        if(this->image != VK_NULL_HANDLE)
        {
            vmaDestroyImage(this->device->native_allocator(), this->image, this->alloc);
            this->image = VK_NULL_HANDLE;
        }
    }

    Image& Image::operator=(Image&& rhs)
    {
        std::swap(this->image, rhs.image);
        std::swap(this->alloc, rhs.alloc);
        std::swap(this->device, rhs.device);
        std::swap(this->width, rhs.width);
        std::swap(this->height, rhs.height);
        std::swap(this->usage, rhs.usage);
        std::swap(this->format, rhs.format);
        std::swap(this->layout, rhs.layout);
        std::swap(this->residency, rhs.residency);
        return *this;
    }

    std::uint32_t Image::get_width() const
    {
        return this->width;
    }

    std::uint32_t Image::get_height() const
    {
        return this->height;
    }

    const LogicalDevice& Image::get_device() const
    {
        return *this->device;
    }

    Image::Format Image::get_format() const
    {
        return this->format;
    }

    Image::Usage Image::get_usage() const
    {
        return this->usage;
    }

    Image::Layout Image::get_layout() const
    {
        return this->layout;
    }

    void Image::set_layout(CommandBufferRecording& recording, Image::Layout new_layout)
    {
        recording.transition_image_layout(*this, new_layout);
        this->layout = new_layout;
    }

    hardware::MemoryResidency Image::get_memory_residency() const
    {
        return this->residency;
    }

    VkImage Image::native() const
    {
        return this->image;
    }
}


#endif // TZ_VULKAN