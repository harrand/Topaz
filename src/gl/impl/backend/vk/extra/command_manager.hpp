#ifndef TOPAZ_GL_IMPL_BACKEND_VK_EXTRA_COMMAND_MANAGER_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK_EXTRA_COMMAND_MANAGER_HPP
#if TZ_VULKAN
#include "gl/impl/backend/vk/command.hpp"
#include <deque>

namespace tz::gl::vk::extra
{
    struct CommandManagerInfo
    {
        const LogicalDevice* device;
        unsigned int pool_size_buffers;
    };

    using CommandBufferHandle = tz::Handle<CommandBuffer>;

    /**
     * Handles vk::CommandPools automagically so you only need to worry about asking for as many vk::CommandBuffers as you need.
     */
    class CommandManager
    {
    public:
        CommandManager(CommandManagerInfo info);
        CommandBufferHandle add_buffer();
        std::size_t pool_count() const;
        std::size_t buffer_count() const;
        const CommandBuffer& operator[](CommandBufferHandle handle) const;
        CommandBuffer& operator[](CommandBufferHandle handle);
        const CommandBuffer* at(CommandBufferHandle handle) const;
        CommandBuffer* at(CommandBufferHandle handle);
        void clear();
        void erase(CommandBufferHandle handle);
    private:
        CommandPool& add_pool();
        CommandBufferHandle to_handle(const CommandPool& parent, std::size_t cmd_buf_id) const;
        std::pair<std::size_t, std::size_t> from_handle(CommandBufferHandle handle) const;
        bool is_freed(CommandBufferHandle handle) const;

        CommandManagerInfo info;
        std::vector<CommandPool> command_pools;
        std::deque<CommandBufferHandle> free_list;
        std::size_t pool_buffer_cursor;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK_EXTRA_COMMAND_MANAGER_HPP