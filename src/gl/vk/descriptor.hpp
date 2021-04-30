#ifndef TOPAZ_GL_VK_DESCRIPTOR_HPP
#define TOPAZ_GL_VK_DESCRIPTOR_HPP
#if TZ_VULKAN
#include "gl/vk/logical_device.hpp"
#include "gl/vk/descriptor_set_layout.hpp"
#include "gl/vk/buffer.hpp"

namespace tz::gl::vk
{
    class DescriptorPool;

    class DescriptorSet
    {
    public:
        friend class DescriptorPool;
        DescriptorSet() = default;
        void initialise(const DescriptorPool& parent, const DescriptorSetLayout& layout, const Buffer& buffer, std::size_t buffer_data_offset = 0);
        VkDescriptorSet native() const;
    private:
        VkDescriptorSet set;
    };

    struct DescriptorPoolSize
    {
        DescriptorType type;
        std::uint32_t descriptor_count;
    };

    using DescriptorPoolSizes = std::initializer_list<DescriptorPoolSize>;

    class DescriptorPool
    {
    public:
        DescriptorPool(const LogicalDevice& device, DescriptorPoolSizes pool_sizes, DescriptorSetLayouts&& layouts);
        DescriptorPool(const DescriptorPool& copy) = delete;
        DescriptorPool(DescriptorPool&& move);
        ~DescriptorPool();

        DescriptorPool& operator=(const DescriptorPool& rhs) = delete;
        DescriptorPool& operator=(DescriptorPool&& rhs);

        const DescriptorSet& operator[](std::size_t index) const;
        DescriptorSet& operator[](std::size_t index);

        template<typename... Args>
        std::size_t with(std::size_t count, std::span<const Buffer> buffers, Args&&... args);

        const LogicalDevice& get_device() const;
    private:
        std::vector<VkDescriptorSet> get_set_natives() const;

        VkDescriptorPool pool;
        const LogicalDevice* device;
        std::vector<DescriptorSet> sets;
        std::vector<DescriptorSetLayout> set_layouts;
    };
}

#include "gl/vk/descriptor.inl"
#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_DESCRIPTOR_HPP