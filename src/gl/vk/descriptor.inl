#if TZ_VULKAN

namespace tz::gl::vk
{
    template<typename... Args>
    std::size_t DescriptorPool::with(std::size_t count, std::span<const Buffer> buffers, Args&&... args)
    {
        auto index = this->sets.size();
        for(std::size_t i = 0; i < count; i++)
        {
            this->sets.emplace_back();
        }
        auto set_natives = this->get_set_natives();
        std::vector<VkDescriptorSetLayout> set_layout_natives;

        for(const auto& layout : this->set_layouts)
        {
            set_layout_natives.push_back(layout.native());
        }

        VkDescriptorSetAllocateInfo alloc{};
        alloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc.descriptorPool = this->pool;
        alloc.descriptorSetCount = count;
        alloc.pSetLayouts = set_layout_natives.data();

        auto res = vkAllocateDescriptorSets(this->device->native(), &alloc, set_natives.data());
        for(std::size_t i = 0; i < count; i++)
        {
            this->sets[i].set = set_natives[i];
            this->sets[i].initialise(*this, this->set_layouts[i], buffers[i], std::forward<Args>(args)...);
        }
        return index;
    }
}

#endif // TZ_VULKAN