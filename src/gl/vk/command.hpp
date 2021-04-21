#ifndef TOPAZ_GL_VK_COMMAND_HPP
#define TOPAZ_GL_VK_COMMAND_HPP
#if TZ_VULKAN
#include "gl/vk/impl/hardware/queue_family.hpp"
#include "gl/vk/impl/setup/logical_device.hpp"

namespace tz::gl::vk
{
    class CommandPool;

    class CommandBuffer
    {
    public:
        friend class CommandPool;
        void begin_recording();
        void end_recording();
        VkCommandBuffer native() const;
        void draw(std::uint32_t vert_count, std::uint32_t inst_count = 0, std::uint32_t first_index = 0, std::uint32_t first_instance = 0);
    private:
        CommandBuffer(const CommandPool& parent){};

        VkCommandBuffer command_buffer;
    };

    class CommandPool
    {
    public:
        CommandPool(const LogicalDevice& device, const hardware::DeviceQueueFamily& queue_family);
        CommandPool(const CommandPool& copy) = delete;
        CommandPool(CommandPool&& move);
        ~CommandPool();

        CommandPool& operator=(const CommandPool& rhs) = delete;
        CommandPool& operator=(CommandPool&& rhs);

        VkCommandPool native() const;
        template<typename... Args>
        std::size_t with(Args&&... args);

        CommandBuffer& operator[](std::size_t idx);
        const CommandBuffer& operator[](std::size_t idx) const;
    private:
        VkCommandPool command_pool;
        const LogicalDevice* device;
        std::vector<CommandBuffer> buffers;
    };
}

#include "gl/vk/command.inl"
#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_COMMAND_HPP