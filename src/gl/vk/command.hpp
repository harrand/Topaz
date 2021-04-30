#ifndef TOPAZ_GL_VK_COMMAND_HPP
#define TOPAZ_GL_VK_COMMAND_HPP
#if TZ_VULKAN
#include "gl/vk/hardware/queue_family.hpp"
#include "gl/vk/logical_device.hpp"
#include "gl/vk/buffer.hpp"
#include "gl/vk/descriptor.hpp"
#include "gl/vk/pipeline/layout.hpp"

namespace tz::gl::vk
{
    class CommandPool;

    class CommandBuffer
    {
    public:
        friend class CommandPool;
        struct OneTimeUseTag{};
        static constexpr OneTimeUseTag OneTimeUse{};

        CommandBuffer(const CommandPool& parent){};
        void begin_recording();
        void begin_recording(OneTimeUseTag onetime_use);
        void end_recording();
        VkCommandBuffer native() const;

        void copy(const Buffer& source, Buffer& destination, std::size_t copy_bytes_length);
        void bind(const Buffer& buf) const;
        void bind(const DescriptorSet& descriptor_set, const pipeline::Layout& layout) const;
        void draw(std::uint32_t vert_count, std::uint32_t inst_count = 1, std::uint32_t first_index = 0, std::uint32_t first_instance = 0);
        void draw_indexed(std::uint32_t index_count, std::uint32_t inst_count = 1, std::uint32_t first_index = 0, std::uint32_t vertex_offset = 0, std::uint32_t first_instance = 0);
        void reset();
    private:
        VkCommandBuffer command_buffer;
    };

    class CommandPool
    {
    public:
        struct RecycleableBufferTag{};
        static constexpr RecycleableBufferTag RecycleBuffer{};

        CommandPool(const LogicalDevice& device, const hardware::DeviceQueueFamily& queue_family);
        CommandPool(const LogicalDevice& device, const hardware::DeviceQueueFamily& queue_family, RecycleableBufferTag recycleable);
        CommandPool(const CommandPool& copy) = delete;
        CommandPool(CommandPool&& move);
        ~CommandPool();

        CommandPool& operator=(const CommandPool& rhs) = delete;
        CommandPool& operator=(CommandPool&& rhs);

        VkCommandPool native() const;
        template<typename... Args>
        std::size_t with(std::size_t count, Args&&... args);
        void clear();

        CommandBuffer& operator[](std::size_t idx);
        const CommandBuffer& operator[](std::size_t idx) const;
    private:
        std::vector<VkCommandBuffer> get_buffer_natives() const;
        
        VkCommandPool command_pool;
        const LogicalDevice* device;
        std::vector<CommandBuffer> buffers;
    };
}

#include "gl/vk/command.inl"
#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_COMMAND_HPP