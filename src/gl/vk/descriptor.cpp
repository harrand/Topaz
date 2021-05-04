#if TZ_VULKAN
#include "gl/vk/descriptor.hpp"
#include <type_traits>

namespace tz::gl::vk
{
    DescriptorPoolBuilder& DescriptorPoolBuilder::with_size(VkDescriptorType type, std::uint32_t descriptor_count)
    {
        VkDescriptorPoolSize& pool_size = this->pool_sizes.emplace_back();
        pool_size.descriptorCount = descriptor_count;
        pool_size.type = type;
        return *this;
    }

    DescriptorPoolBuilder& DescriptorPoolBuilder::with_layout(const DescriptorSetLayout& layout)
    {
        this->layouts.push_back(layout.native());
        return *this;
    }

    DescriptorPoolBuilder& DescriptorPoolBuilder::with_capacity(std::uint32_t max_sets)
    {
        this->max_sets = max_sets;
        return *this;
    }

    void DescriptorSetsCreationRequest::add_buffer(const Buffer& buffer, VkDeviceSize offset, VkDeviceSize range, std::uint32_t binding_id)
    {
        auto& info = std::get<VkDescriptorBufferInfo>(this->resources.emplace_back(VkDescriptorBufferInfo{}));
        info.buffer = buffer.native();
        info.offset = offset;
        info.range = range;
        switch(buffer.get_type())
        {
            case BufferType::Uniform:
                this->types.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
            break;
            default:
                tz_error("Given a buffer to attach to a descriptor set, but the buffer type is not compatible.");
            break;
        }
        this->bindings.push_back(binding_id);
    }

    VkDescriptorSet DescriptorSet::native() const
    {
        return this->set;
    }

    DescriptorSet::DescriptorSet(VkDescriptorSet set):
    set(set){}

    DescriptorPool::DescriptorPool(const LogicalDevice& device, const DescriptorPoolBuilder& builder):
    pool(VK_NULL_HANDLE),
    layouts(builder.layouts),
    sets(),
    device(&device)
    {
        VkDescriptorPoolCreateInfo create{};
        create.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        create.poolSizeCount = builder.pool_sizes.size();
        create.pPoolSizes = builder.pool_sizes.data();
        create.maxSets = builder.max_sets;

        auto res = vkCreateDescriptorPool(this->device->native(), &create, nullptr, &this->pool);
        tz_assert(res == VK_SUCCESS, "Failed to create descriptor pool");
    }

    DescriptorPool::DescriptorPool(DescriptorPool&& move):
    pool(VK_NULL_HANDLE),
    layouts(),
    sets(),
    device(nullptr)
    {
        *this = std::move(move);
    }

    DescriptorPool::~DescriptorPool()
    {
        if(this->pool != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(this->device->native(), this->pool, nullptr);
            this->pool = VK_NULL_HANDLE;
        }
        this->layouts.clear();
    }

    DescriptorPool& DescriptorPool::operator=(DescriptorPool&& rhs)
    {
        std::swap(this->pool, rhs.pool);
        std::swap(this->layouts, rhs.layouts);
        std::swap(this->sets, rhs.sets);
        std::swap(this->device, rhs.device);
        return *this;
    }

    void DescriptorPool::initialise_sets(DescriptorSetsCreationRequest request)
    {
        if(!this->sets.empty())
        {
            this->destroy_sets();
        }

        // Allocate the sets
        VkDescriptorSetAllocateInfo alloc{};
        alloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc.pNext = nullptr;
        alloc.descriptorPool = this->pool;
        alloc.descriptorSetCount = request.resources.size();
        alloc.pSetLayouts = this->layouts.data();
        this->sets.resize(request.resources.size());
        auto res = vkAllocateDescriptorSets(this->device->native(), &alloc, this->sets.data());
        tz_assert(res == VK_SUCCESS, "Failed to allocate descriptor sets");

        // Then initialise them.
        std::vector<VkWriteDescriptorSet> writes;
        for(std::size_t i = 0; i < request.resources.size(); i++)
        {
            VkWriteDescriptorSet& write = writes.emplace_back();
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.pNext = nullptr;
            write.dstSet = this->sets[i];
            write.descriptorCount = 1;
            write.descriptorType = request.types[i];
            DescriptorSetsCreationRequest::ResourceInfoVariant resource_info = request.resources[i];
            std::visit([&write, &resource_info](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr(std::is_same_v<T, VkDescriptorBufferInfo>)
                {
                    write.pBufferInfo = &std::get<VkDescriptorBufferInfo>(resource_info);
                }
                else
                {
                    static_assert(std::is_void_v<T>, "DescriptorSetsCreationRequest::ResourceInfoVariant hasn't been updated, or resource type is not supported.");
                }
            }, resource_info);
            write.dstArrayElement = 0;
            write.dstBinding = request.bindings[i];
        }

        vkUpdateDescriptorSets(this->device->native(), writes.size(), writes.data(), 0, nullptr);
    }

    DescriptorSet DescriptorPool::operator[](std::size_t index) const
    {
        return {this->sets[index]};
    }

    void DescriptorPool::destroy_sets()
    {
        vkFreeDescriptorSets(this->device->native(), this->pool, this->sets.size(), this->sets.data());
        this->sets.clear();
    }
}

#endif // TZ_VULKAN