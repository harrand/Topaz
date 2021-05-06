#ifndef TOPAZ_GL_VK_DESCRIPTOR_HPP
#define TOPAZ_GL_VK_DESCRIPTOR_HPP
#if TZ_VULKAN
#include "gl/vk/logical_device.hpp"
#include "gl/vk/descriptor_set_layout.hpp"
#include "gl/vk/buffer.hpp"
#include "gl/vk/image_view.hpp"
#include "gl/vk/sampler.hpp"
#include <variant>

namespace tz::gl::vk
{
    class DescriptorPoolBuilder
    {
    public:
        DescriptorPoolBuilder() = default;
        DescriptorPoolBuilder& with_size(VkDescriptorType type, std::uint32_t descriptor_count);
        DescriptorPoolBuilder& with_layout(const DescriptorSetLayout& layout);
        DescriptorPoolBuilder& with_capacity(std::uint32_t max_sets);

        friend class DescriptorPool;
    private:
        std::vector<VkDescriptorPoolSize> pool_sizes;
        std::vector<VkDescriptorSetLayout> layouts;
        std::uint32_t max_sets;
    };

    class DescriptorSetsCreationRequest
    {
    public:
        DescriptorSetsCreationRequest() = default;
        void add_buffer(const Buffer& buffer, VkDeviceSize offset, VkDeviceSize range, std::uint32_t binding_id);
        void add_image(const ImageView& image, const Sampler& sampler, VkDeviceSize offset, VkDeviceSize range, std::uint32_t binding_id);;
        friend class DescriptorPool;
    private:
        using ResourceInfoVariant = std::variant<VkDescriptorBufferInfo, VkDescriptorImageInfo>;
        std::vector<ResourceInfoVariant> resources;
        std::vector<VkDescriptorType> types;
        std::vector<std::uint32_t> bindings;
    };

    class DescriptorSetsCreationRequests
    {
    public:
        DescriptorSetsCreationRequests() = default;
        DescriptorSetsCreationRequest& new_request();
        std::size_t size() const;
        const DescriptorSetsCreationRequest& operator[](std::size_t i) const;
        DescriptorSetsCreationRequest& operator[](std::size_t i);
        auto begin();
        auto end();
    private:
        std::vector<DescriptorSetsCreationRequest> requests;
    };

    class DescriptorSet
    {
    public:
        VkDescriptorSet native() const;
        friend class DescriptorPool;
    private:
        DescriptorSet(VkDescriptorSet set);
        VkDescriptorSet set;
    };

    class DescriptorPool
    {
    public:
        DescriptorPool(const LogicalDevice& device, const DescriptorPoolBuilder& builder);
        DescriptorPool(const DescriptorPool& copy) = delete;
        DescriptorPool(DescriptorPool&& move);
        ~DescriptorPool();

        DescriptorPool& operator=(const DescriptorPool& rhs) = delete;
        DescriptorPool& operator=(DescriptorPool&& rhs);

        void initialise_sets(DescriptorSetsCreationRequests requests);
        DescriptorSet operator[](std::size_t index) const;
    private:
        void initialise_set(DescriptorSetsCreationRequest request, std::size_t set_id);
        void destroy_sets();

        VkDescriptorPool pool;
        std::vector<VkDescriptorSetLayout> layouts;
        std::vector<VkDescriptorSet> sets;
        const LogicalDevice* device;
    };
}

#include "gl/vk/descriptor.inl"
#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_DESCRIPTOR_HPP