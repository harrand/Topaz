#if TZ_VULKAN
#include "gl/vk/image.hpp"
#include "gl/vk/command.hpp"

namespace tz::gl::vk
{
    Image::Image(const LogicalDevice& device, std::uint32_t width, std::uint32_t height, Image::Format format, Image::UsageField usage, hardware::MemoryModule resource_memory):
    image(VK_NULL_HANDLE),
    image_memory(VK_NULL_HANDLE),
    device(&device),
    width(width),
    height(height),
    format(format),
    layout(Image::Layout::Undefined)
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
        create.usage = static_cast<VkImageUsageFlags>(static_cast<Image::Usage>(usage));
        create.samples = VK_SAMPLE_COUNT_1_BIT;
        create.flags = 0;

        auto res = vkCreateImage(this->device->native(), &create, nullptr, &this->image);
        tz_assert(res == VK_SUCCESS, "Failed to create image");

        VkMemoryRequirements mem_reqs;
        vkGetImageMemoryRequirements(this->device->native(), this->image, &mem_reqs);

        VkMemoryAllocateInfo alloc{};
        alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc.allocationSize = mem_reqs.size;
        alloc.memoryTypeIndex = resource_memory.index;

        res = vkAllocateMemory(this->device->native(), &alloc, nullptr, &this->image_memory);
        tz_assert(res == VK_SUCCESS, "Failed to allocate device memory for image");
        vkBindImageMemory(this->device->native(), this->image, this->image_memory, 0);
    }

    Image::Image(Image&& move):
    image(VK_NULL_HANDLE),
    image_memory(VK_NULL_HANDLE),
    device(nullptr),
    width(0),
    height(0)
    {
        *this = std::move(move);
    }

    Image::~Image()
    {
        if(this->image != VK_NULL_HANDLE)
        {
            vkDestroyImage(this->device->native(), this->image, nullptr);
            this->image = VK_NULL_HANDLE;
        }
        if(this->image_memory != VK_NULL_HANDLE)
        {
            vkFreeMemory(this->device->native(), this->image_memory, nullptr);
            this->image_memory = VK_NULL_HANDLE;
        }
    }

    Image& Image::operator=(Image&& rhs)
    {
        std::swap(this->image, rhs.image);
        std::swap(this->image_memory, rhs.image_memory);
        std::swap(this->device, rhs.device);
        std::swap(this->width, rhs.width);
        std::swap(this->height, rhs.height);
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

    Image::Format Image::get_format() const
    {
        return this->format;
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

    VkImage Image::native() const
    {
        return this->image;
    }
}


#endif // TZ_VULKAN