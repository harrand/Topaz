#if TZ_VULKAN
#include "gl/impl/backend/vk/descriptor_set_layout.hpp"

namespace tz::gl::vk
{
    std::uint32_t LayoutBuilder::add(DescriptorType type, pipeline::ShaderTypeField relevant_stages)
    {
        VkDescriptorType t;
        switch(type)
        {
            case DescriptorType::UniformBuffer:
                t = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            break;
            case DescriptorType::CombinedImageSampler:
                t = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            break;
            default:
                tz_error("Unrecognised descriptor type");
            break;
        }
        this->binding_types.push_back(t);
        if(relevant_stages == pipeline::ShaderTypeField::All())
        {
            this->binding_relevant_shader_stages.push_back(VK_SHADER_STAGE_ALL_GRAPHICS);
        }
        else
        {
            if(relevant_stages.contains(pipeline::ShaderType::Vertex))
            {
                this->binding_relevant_shader_stages.push_back(VK_SHADER_STAGE_VERTEX_BIT);
            }
            else if(relevant_stages.contains(pipeline::ShaderType::Fragment))
            {
                this->binding_relevant_shader_stages.push_back(VK_SHADER_STAGE_FRAGMENT_BIT);
            }
            else
            {
                tz_error("Unrecognised shader type field.");
            }
        }
        return this->binding_types.size() - 1;
    }

    VkDescriptorSetLayoutBinding LayoutBuilder::operator[](std::size_t index) const
    {
        VkDescriptorSetLayoutBinding binding{};
        binding.binding = index;
        binding.descriptorCount = 1;
        binding.descriptorType = this->binding_types[index];
        binding.stageFlags = this->binding_relevant_shader_stages[index];
        binding.pImmutableSamplers = nullptr;
        return binding;
    }

    VkDescriptorSetLayoutCreateInfo LayoutBuilder::native() const
    {
        VkDescriptorSetLayoutCreateInfo layout{};
        layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout.pNext = nullptr;
        layout.bindingCount = this->size();
        for(std::size_t i = 0; i < this->size(); i++)
        {
            this->bindings.push_back((*this)[i]);
        }
        layout.pBindings = this->bindings.data();
        return layout;
    }

    std::size_t LayoutBuilder::size() const
    {
        tz_assert(this->binding_types.size() == this->binding_relevant_shader_stages.size(), "Layout Builder is corrupted");
        return this->binding_types.size();
    }

    DescriptorSetLayout::DescriptorSetLayout(const LogicalDevice& device, const LayoutBuilder& builder):
    layout(VK_NULL_HANDLE),
    device(&device)
    {
        VkDescriptorSetLayoutCreateInfo layout_create = builder.native();
        auto res = vkCreateDescriptorSetLayout(this->device->native(), &layout_create, nullptr, &this->layout);
        tz_assert(res == VK_SUCCESS, "Failed to create descriptor set layout");
    }

    DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& move):
    layout(VK_NULL_HANDLE),
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
        std::swap(this->device, rhs.device);
        return *this;
    }

    VkDescriptorSetLayout DescriptorSetLayout::native() const
    {
        return this->layout;
    }
}

#endif // TZ_VULKAN