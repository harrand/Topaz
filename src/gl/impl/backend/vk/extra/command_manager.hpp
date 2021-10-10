#ifndef TOPAZ_GL_IMPL_BACKEND_VK_EXTRA_COMMAND_MANAGER_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK_EXTRA_COMMAND_MANAGER_HPP
#if TZ_VULKAN
#include "gl/impl/backend/vk/command.hpp"
#include <unordered_set>

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
		/**
		 * Request a new buffer. You should assume the returned buffer is in a fully resetted state.
		 * 
		 * @note This buffer may or may not be recycled from a previous @ref CommandManager::erase.
		 * @return Handle to a new CommandBuffer.
		 */
		CommandBufferHandle add_buffer();
		/**
		 * Retrieve the number of CommandPools currently in-flight.
		 * @return Number of CommandPools.
		 */
		std::size_t pool_count() const;
		/**
		 * Retrieve the number of CommandBuffers currently being managed.
		 * @return Number of CommandBuffers.
		 */
		std::size_t buffer_count() const;
		/**
		 * Retrieve a CommandBuffer from a given handle.
		 * 
		 * If the handle is not valid, or corresponds to a buffer that was deleted, the behaviour is undefined. See @ref CommandManager::at if you're not absolutely certain the handle is valid.
		 * @return Reference to the CommandBuffer corresponding to the handle.
		 */
		const CommandBuffer& operator[](CommandBufferHandle handle) const;
		/**
		 * Retrieve a CommandBuffer from a given handle.
		 * 
		 * If the handle is not valid, or corresponds to a buffer that was deleted, the behaviour is undefined. See @ref CommandManager::at if you're not absolutely certain the handle is valid.
		 * @return Reference to the CommandBuffer corresponding to the handle.
		 */
		CommandBuffer& operator[](CommandBufferHandle handle);
		/**
		 * Retrieve a CommandBuffer from a given handle, if it exists.
		 * 
		 * @return Reference to the CommandBuffer corresponding to the handle, or nullptr if no such CommandBuffer exists.
		 */
		const CommandBuffer* at(CommandBufferHandle handle) const;
		/**
		 * Retrieve a CommandBuffer from a given handle, if it exists.
		 * 
		 * @return Reference to the CommandBuffer corresponding to the handle, or nullptr if no such CommandBuffer exists.
		 */
		CommandBuffer* at(CommandBufferHandle handle);
		/**
		 * Erase all CommandBuffers being managed.
		 */
		void clear();
		/**
		 * Erase the CommandBuffer corresponding to the given handle.
		 * 
		 * If the handle is not valid, or corresponds to a buffer that was deleted, the behaviour is undefined.
		 */
		void erase(CommandBufferHandle handle);
	private:
		CommandPool& add_pool();
		std::size_t buffer_capacity() const;
		CommandBufferHandle to_handle(const CommandPool& parent, std::size_t cmd_buf_id) const;
		std::pair<std::size_t, std::size_t> from_handle(CommandBufferHandle handle) const;
		bool is_freed(CommandBufferHandle handle) const;

		CommandManagerInfo info;
		std::vector<CommandPool> command_pools;
		std::unordered_set<CommandBufferHandle> free_list;
		std::size_t pool_buffer_cursor;
	};
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK_EXTRA_COMMAND_MANAGER_HPP