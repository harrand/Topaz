#ifndef TOPAZ_GL_VK_COMMAND_HPP
#define TOPAZ_GL_VK_COMMAND_HPP
#if TZ_VULKAN
#include "core/containers/polymorphic_list.hpp"
#include "core/handle.hpp"
#include "gl/impl/backend/vk/hardware/queue_family.hpp"
#include "gl/impl/backend/vk/logical_device.hpp"
#include "gl/impl/backend/vk/buffer.hpp"
#include "gl/impl/backend/vk/descriptor.hpp"
#include "gl/impl/backend/vk/pipeline/layout.hpp"
#include "gl/impl/backend/vk/image.hpp"
#include <functional>

namespace tz::gl::vk
{
	class CommandPool;
	class CommandBuffer;

	enum class CommandType
	{
		BufferCopyBuffer,
		BufferCopyImage,
		TransitionImageLayout,
		BindBuffer,
		BindDescriptorSet,
		Draw,
		DrawIndexed,
		DrawIndirect,
		Dispatch
	};

	class ICommand
	{
	public:
		constexpr ICommand() = default;
		virtual ~ICommand() = default;
		constexpr virtual CommandType get_type() const = 0;
		virtual void operator()() const = 0;
	};

	template<CommandType C>
	class Command : public ICommand
	{
	public:
		constexpr Command() = default;
		constexpr virtual CommandType get_type() const final{return C;}
		virtual void operator()() const final{}
	};

	template<>
	class Command<CommandType::TransitionImageLayout> : public ICommand
	{
	public:
		Command(Image& image, Image::Layout new_layout);

		constexpr CommandType get_type() const{return CommandType::TransitionImageLayout;}
		virtual void operator()() const final;
	private:
		Image* image;
		Image::Layout new_layout;
	};

	class CommandBufferRecording
	{
	public:
		CommandBufferRecording(const CommandBufferRecording& copy) = delete;
		CommandBufferRecording(CommandBufferRecording&& move);
		~CommandBufferRecording();
		CommandBufferRecording& operator=(const CommandBufferRecording& rhs) = delete;
		CommandBufferRecording& operator=(CommandBufferRecording&& rhs);

		void buffer_copy_buffer(const Buffer& source, Buffer& destination, std::size_t copy_bytes_length);
		void buffer_copy_image(const Buffer& source, const Image& destination);
		void transition_image_layout(Image& image, Image::Layout new_layout);
		void bind(const Buffer& buf);
		void bind(const DescriptorSet& descriptor_set, const pipeline::Layout& layout);
		void bind_compute(const DescriptorSet& descriptor_set, const pipeline::Layout& layout);
		void draw(std::uint32_t vertex_count, std::uint32_t instance_count = 1, std::uint32_t first_index = 0, std::uint32_t first_instance = 0);
		void draw_indexed(std::uint32_t index_count, std::uint32_t instance_count = 1, std::uint32_t first_index = 0, std::uint32_t vertex_offset = 0, std::uint32_t first_instance = 0);
		void draw_indirect(const vk::Buffer& draw_indirect_buffer, std::uint32_t draw_count);
		void dispatch(std::uint32_t group_count_x, std::uint32_t group_count_y, std::uint32_t group_count_z);

		friend class CommandBuffer;
	private:
		CommandBufferRecording(const CommandBuffer& buffer, std::function<void(CommandBufferRecording&)> on_recording_end);
		const CommandBuffer* command_buffer;
		std::function<void(CommandBufferRecording&)> on_recording_end;
		tz::PolymorphicList<ICommand> recorded_commands;
	};

	class CommandBuffer
	{
	public:
		friend class CommandPool;
		struct OneTimeUseTag{};
		static constexpr OneTimeUseTag OneTimeUse{};

		CommandBuffer(const CommandPool& parent);
		CommandBufferRecording record();
		VkCommandBuffer native() const;

		void reset();
		void operator()() const;
		bool operator==(const CommandBuffer& rhs) const;
	private:
		void notify_recording_begin();
		void notify_recording_end(CommandBufferRecording& recording);
		VkCommandBuffer command_buffer;
		bool currently_recording;
		tz::PolymorphicList<ICommand> recorded_commands;
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
		bool empty() const;
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

#include "gl/impl/backend/vk/command.inl"
#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_COMMAND_HPP