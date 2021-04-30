#if TZ_VULKAN
#include "gl/vk/descriptor_set_layout.hpp"

namespace tz::gl::vk
{
    DescriptorSetLayout::DescriptorSetLayout(const LogicalDevice& device, DescriptorSetLayoutBinding binding):
    layout(VK_NULL_HANDLE),
    binding(binding),
    device(&device)
    {
        VkDescriptorSetLayoutCreateInfo create{};
        create.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        create.bindingCount = 1;

        VkDescriptorSetLayoutBinding bind;
        bind.binding = binding.binding;
        bind.descriptorCount = 1;
        switch(binding.type)
        {
            case DescriptorType::UniformBuffer:
                bind.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            break;
            default:
                tz_error("Unknown descriptor type");
            break;
        }
        // TODO: Selective instead of allowing it to be used in all stages?
        bind.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
        // TODO: Image sampling related descriptors?
        bind.pImmutableSamplers = nullptr;

        create.pBindings = &bind;

        auto res = vkCreateDescriptorSetLayout(this->device->native(), &create, nullptr, &this->layout);
        tz_assert(res == VK_SUCCESS, "Failed to create descriptor set layout");
    }

    DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& move):
    layout(VK_NULL_HANDLE),
    binding(),
    device(nullptr)
    {
        *this = std::move(move);
    }

    DescriptorSetLayout::~DescriptorSetLayout()
    {
        if(this->device != nullptr)
        {
            vkDestroyDescriptorSetLayout(this->device->native(), this->layout, nullptr);
            this->layout = VK_NULL_HANDLE;
        }
    }

    DescriptorSetLayout& DescriptorSetLayout::operator=(DescriptorSetLayout&& rhs)
    {
        std::swap(this->layout, rhs.layout);
        std::swap(this->binding, rhs.binding);
        std::swap(this->device, rhs.device);
        return *this;
    }

    VkDescriptorSetLayout DescriptorSetLayout::native() const
    {
        return this->layout;
    }

    DescriptorSetLayoutBinding DescriptorSetLayout::get_binding() const
    {
        return this->binding;
    }
}

#endif // TZ_VULKAN