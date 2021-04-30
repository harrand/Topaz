#if TZ_VULKAN
#include "gl/vk/descriptor.hpp"

namespace tz::gl::vk
{
    void DescriptorSet::initialise(const DescriptorPool& parent, const DescriptorSetLayout& layout, const Buffer& buffer, std::size_t buffer_data_offset)
    {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = buffer.native();
        buffer_info.offset = buffer_data_offset;
        // TODO: Not whole thing
        buffer_info.range = VK_WHOLE_SIZE;
        
        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = this->set;
        auto binding = layout.get_binding();
        write.dstBinding = binding.binding;
        write.dstArrayElement = 0;

        switch(binding.type)
        {
            case DescriptorType::UniformBuffer:
                write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            break;
            default:
                tz_error("Unknown descriptor type");
            break;
        }
        write.descriptorCount = 1;
        write.pBufferInfo = &buffer_info;
        write.pImageInfo = nullptr;
        write.pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(parent.get_device().native(), 1, &write, 0, nullptr);
    }

    VkDescriptorSet DescriptorSet::native() const
    {
        return this->set;
    }

    DescriptorPool::DescriptorPool(const LogicalDevice& device, DescriptorPoolSizes pool_sizes, DescriptorSetLayouts&& layouts):
    pool(VK_NULL_HANDLE),
    device(&device),
    sets(),
    set_layouts(std::move(layouts))
    {
        std::uint32_t max_sets = 0;
        std::vector<VkDescriptorPoolSize> pool_size_natives;
        for(const auto& pool_size : pool_sizes)
        {
            max_sets = std::max(max_sets, pool_size.descriptor_count);
            VkDescriptorPoolSize& pool_native = pool_size_natives.emplace_back();
            switch(pool_size.type)
            {
                case DescriptorType::UniformBuffer:
                    pool_native.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                break;
                default:
                    tz_error("Unrecognised descriptor type");
                break;
            }
            pool_native.descriptorCount = pool_size.descriptor_count;
        }
        VkDescriptorPoolCreateInfo create{};
        create.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        create.poolSizeCount = pool_sizes.size();
        create.pPoolSizes = pool_size_natives.data();
        // TODO: Fix. This is almost certainly wrong
        create.maxSets = max_sets;

        auto res = vkCreateDescriptorPool(this->device->native(), &create, nullptr, &this->pool);
        tz_assert(res == VK_SUCCESS, "Failed to create descriptor pool");
    }

    DescriptorPool::DescriptorPool(DescriptorPool&& move):
    pool(VK_NULL_HANDLE),
    device(nullptr),
    sets(),
    set_layouts()
    {
        *this = std::move(move);
    }

    DescriptorPool::~DescriptorPool()
    {
        if(this->device != nullptr)
        {
            vkDestroyDescriptorPool(this->device->native(), this->pool, nullptr);
            this->pool = VK_NULL_HANDLE;
        }
    }

    DescriptorPool& DescriptorPool::operator=(DescriptorPool&& rhs)
    {
        std::swap(this->pool, rhs.pool);
        std::swap(this->device, rhs.device);
        std::swap(this->sets, rhs.sets);
        std::swap(this->set_layouts, rhs.set_layouts);
        return *this;
    }

    const DescriptorSet& DescriptorPool::operator[](std::size_t index) const
    {
        tz_assert(this->sets.size() > index, "Index %zu invalid for set list of size %zu", index, this->sets.size());
        return this->sets[index];
    }

    DescriptorSet& DescriptorPool::operator[](std::size_t index)
    {
        tz_assert(this->sets.size() > index, "Index %zu invalid for set list of size %zu", index, this->sets.size());
        return this->sets[index];
    }

    const LogicalDevice& DescriptorPool::get_device() const
    {
        return *this->device;
    }

    std::vector<VkDescriptorSet> DescriptorPool::get_set_natives() const
    {
        std::vector<VkDescriptorSet> set_natives;
        for(const auto& set : this->sets)
        {
            set_natives.push_back(set.native());
        }
        return set_natives;
    }
}

#endif // TZ_VULKAN