#if TZ_VULKAN
#include "gl/impl/backend/vk/extra/command_manager.hpp"

namespace tz::gl::vk::extra
{
    CommandManager::CommandManager(CommandManagerInfo info):
    info(info),
    command_pools(),
    free_list(),
    pool_buffer_cursor(0)
    {

    }

    CommandBufferHandle CommandManager::add_buffer()
    {
        // Firstly, our free-list might have a buffer available.
        {
            if(!this->free_list.empty())
            {
                CommandBufferHandle handle = this->free_list.front();
                // Reset the buffer just incase.
                (*this)[handle].reset();
                this->free_list.pop_front();
                return handle;
            }
        }

        // Retrieve a pointer to the latest pool.
        CommandPool* pool = nullptr;
        if(this->command_pools.empty() || this->pool_buffer_cursor == 0)
        {
            // If there are no pools, or the most recent pool is full, make a new one.
            pool = &this->add_pool();
        }
        else
        {
            // We have a pool, but it might now be full.
            pool = &this->command_pools.back();
        }

        // Create the buffer.
        pool->with(1);

        // Retrieve a handle.
        CommandBufferHandle ret = this->to_handle(*pool, pool_buffer_cursor);

        // Advance the cursor forward. Circle around if we're full.
        pool_buffer_cursor++;
        if(pool_buffer_cursor >= this->info.pool_size_buffers)
        {
            pool_buffer_cursor = 0;
        }

        return ret;
    }

    std::size_t CommandManager::pool_count() const
    {
        return this->command_pools.size();
    }

    std::size_t CommandManager::buffer_count() const
    {
        if(this->command_pools.empty())
        {
            return 0;
        }
        if(this->pool_buffer_cursor == 0)
        {
            // All pools are full.
            return (this->command_pools.size() * this->info.pool_size_buffers) - this->free_list.size();
        }
        return ((this->command_pools.size() - 1) * this->info.pool_size_buffers) + pool_buffer_cursor - this->free_list.size();
    }

    const CommandBuffer& CommandManager::operator[](CommandBufferHandle handle) const
    {
        auto pair = this->from_handle(handle);
        return this->command_pools[pair.first][pair.second];
    }

    CommandBuffer& CommandManager::operator[](CommandBufferHandle handle)
    {
        auto pair = this->from_handle(handle);
        return this->command_pools[pair.first][pair.second];
    }

    const CommandBuffer* CommandManager::at(CommandBufferHandle handle) const
    {
        if(this->is_freed(handle))
        {
            return nullptr;
        }
        auto pair = this->from_handle(handle);
        if(this->command_pools.size() <= pair.first)
        {
            return nullptr;
        }
        if(this->info.pool_size_buffers <= pair.second)
        {
            return nullptr;
        }
        return &this->command_pools[pair.first][pair.second];
    }

    CommandBuffer* CommandManager::at(CommandBufferHandle handle)
    {
        if(this->is_freed(handle))
        {
            return nullptr;
        }
        auto pair = this->from_handle(handle);
        if(this->command_pools.size() <= pair.first)
        {
            return nullptr;
        }
        if(this->info.pool_size_buffers <= pair.second)
        {
            return nullptr;
        }
        return &this->command_pools[pair.first][pair.second];
    }

    void CommandManager::clear()
    {
        this->command_pools.clear();
        this->pool_buffer_cursor = 0;
    }

    void CommandManager::erase(CommandBufferHandle handle)
    {
        this->free_list.push_back(handle);
    }

    CommandPool& CommandManager::add_pool()
    {
        this->pool_buffer_cursor = 0;
        return this->command_pools.emplace_back(*this->info.device, this->info.device->get_queue_family(), CommandPool::RecycleableBufferTag{});
    }

    CommandBufferHandle CommandManager::to_handle(const CommandPool& parent, std::size_t cmd_buf_id) const
    {
        const unsigned int buffers_per_pool = this->info.pool_size_buffers;
        std::size_t cumulative_buffer_id = buffers_per_pool * std::distance(this->command_pools.data(), &parent);
        cumulative_buffer_id += cmd_buf_id;
        return {static_cast<tz::HandleValue>(cumulative_buffer_id)};
    }

    std::pair<std::size_t, std::size_t> CommandManager::from_handle(CommandBufferHandle handle) const
    {
        auto hand_val = static_cast<std::size_t>(static_cast<tz::HandleValue>(handle));
        std::size_t pool_id = hand_val / this->info.pool_size_buffers;
        return {pool_id, hand_val - pool_id * this->info.pool_size_buffers};
    }

    bool CommandManager::is_freed(CommandBufferHandle handle) const
    {
        return std::find(this->free_list.begin(), this->free_list.end(), handle) != this->free_list.end();
    }
}

#endif // TZ_VULKAN