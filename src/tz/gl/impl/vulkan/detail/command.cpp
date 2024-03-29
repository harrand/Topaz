#if TZ_VULKAN
#include "tz/gl/impl/vulkan/detail/command.hpp"
#include <numeric>

namespace tz::gl::vk2
{
	CommandBufferRecording::DynamicRenderingRun::DynamicRenderingRun(CommandBufferRecording& record, std::span<const vk2::ImageView> colour_attachments, const vk2::ImageView* depth_attachment, DynamicRenderingRunInfo rinfo):
	recording(&record)
	{
		tz::vec2ui dims = tz::vec2ui::zero();
		std::vector<VkRenderingAttachmentInfo> colour_attachment_natives(colour_attachments.size());
		std::transform(colour_attachments.begin(), colour_attachments.end(), colour_attachment_natives.begin(),
		[rinfo](const auto& imgview)->VkRenderingAttachmentInfo
		{
			VkClearColorValue clearval;
			clearval.float32[0] = rinfo.clear_colour[0];
			clearval.float32[1] = rinfo.clear_colour[1];
			clearval.float32[2] = rinfo.clear_colour[2];
			clearval.float32[3] = rinfo.clear_colour[3];
			return
			{
				.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
				.pNext = nullptr,
				.imageView = imgview.native(),
				.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				.resolveMode = VK_RESOLVE_MODE_NONE,
				.resolveImageView = VK_NULL_HANDLE,
				.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.loadOp = static_cast<VkAttachmentLoadOp>(rinfo.colour_load),
				.storeOp = static_cast<VkAttachmentStoreOp>(rinfo.colour_store),
				.clearValue =
				{
					.color = {clearval}
				}
			};
		});
		if(colour_attachments.size())
		{
			dims = colour_attachments.front().get_image().get_dimensions();
		}
		tz::assert(std::all_of(colour_attachments.begin(), colour_attachments.end(), [dims](const auto& imgview){return imgview.get_image().get_dimensions() == dims;}));
		VkClearDepthStencilValue clearvald;
		clearvald.depth = 1.0f;
		VkRenderingAttachmentInfo depth_attachment_native
		{
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.pNext = nullptr,
			.imageView = VK_NULL_HANDLE,
			.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			.resolveMode = VK_RESOLVE_MODE_NONE,
			.resolveImageView = VK_NULL_HANDLE,
			.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.loadOp = static_cast<VkAttachmentLoadOp>(rinfo.depth_load),
			.storeOp = static_cast<VkAttachmentStoreOp>(rinfo.depth_store),
			.clearValue = 
			{
				.depthStencil = {clearvald}
			}
		};
		if(depth_attachment != nullptr)
		{
			depth_attachment_native.imageView = depth_attachment->native();
			#if TZ_DEBUG
				auto ddims = depth_attachment->get_image().get_dimensions();
				tz::assert(ddims == dims, "Depth attachment dimensions (%ux%u) does not match the dimensions of the render area derived from colour attachments (%ux%u)", ddims[0], ddims[1], dims[0], dims[1]);
			#endif // TZ_DEBUG
		}
		auto dims32 = static_cast<tz::vec2ui32>(dims);
		VkRenderingInfo info =
		{
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
			.pNext = nullptr,
			.flags = 0,
			.renderArea =
			{
				.offset = {0, 0},
				.extent = {dims32[0], dims32[1]}
			},
			.layerCount = 1,
			.viewMask = 0,
			.colorAttachmentCount = static_cast<std::uint32_t>(colour_attachment_natives.size()),
			.pColorAttachments = colour_attachment_natives.size() ? colour_attachment_natives.data() : nullptr,
			.pDepthAttachment = (depth_attachment != nullptr) ? &depth_attachment_native : nullptr,
			.pStencilAttachment = nullptr
		};
		vkCmdBeginRendering(this->recording->get_command_buffer().native(), &info);
		this->recording->register_command(VulkanCommand::BeginDynamicRendering{});
	}

	CommandBufferRecording::DynamicRenderingRun::~DynamicRenderingRun()
	{
		vkCmdEndRendering(this->recording->get_command_buffer().native());
		this->recording->register_command(VulkanCommand::EndDynamicRendering{});
	}

	CommandBufferRecording::CommandBufferRecording(CommandBuffer& command_buffer):
	command_buffer(&command_buffer)
	{
		VkCommandBufferBeginInfo begin
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pInheritanceInfo = nullptr
		};
		VkResult res = vkBeginCommandBuffer(this->command_buffer->native(), &begin);
		switch(res)
		{
			case VK_SUCCESS:
				this->command_buffer->set_recording(true);
			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				tz::error("Failed to create CommandBufferRecording because we ran out of host memory (RAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				tz::error("Failed to create CommandBufferRecording because we ran out of device memory (VRAM). Please ensure that your system meets the minimum requirements.");
			break;
			default:
				tz::error("Failed to create CommandBufferRecording but cannot determine why. Please submit a bug report.");
			break;
		}
	}

	CommandBufferRecording::CommandBufferRecording(CommandBufferRecording&& move):
	command_buffer(nullptr)
	{
		*this = std::move(move);
	}

	CommandBufferRecording::~CommandBufferRecording()
	{
		if(this->command_buffer != nullptr)
		{
			vkEndCommandBuffer(this->command_buffer->native());
			this->command_buffer->set_recording(false);
		}
	}

	CommandBufferRecording& CommandBufferRecording::operator=(CommandBufferRecording&& rhs)
	{
		std::swap(this->command_buffer, rhs.command_buffer);
		return *this;
	}

	void CommandBufferRecording::bind_pipeline(VulkanCommand::BindPipeline command)
	{
		tz::assert(command.pipeline != nullptr, "BindPipeline Command contained nullptr GraphicsPipeline");
		this->register_command(command);

		vkCmdBindPipeline(this->get_command_buffer().native(), static_cast<VkPipelineBindPoint>(command.pipeline->get_context()), command.pipeline->native());
	}

	void CommandBufferRecording::dispatch(VulkanCommand::Dispatch command)
	{
		this->register_command(command);
		vkCmdDispatch(this->get_command_buffer().native(), command.groups[0], command.groups[1], command.groups[2]);
	}

	void CommandBufferRecording::draw(VulkanCommand::Draw command)
	{
		this->register_command(command);
		vkCmdDraw(this->get_command_buffer().native(), command.vertex_count, command.instance_count, command.first_vertex, command.first_instance);
	}

	void CommandBufferRecording::draw_indexed(VulkanCommand::DrawIndexed command)
	{
		this->register_command(command);
		vkCmdDrawIndexed(this->get_command_buffer().native(), command.index_count, command.instance_count, command.first_index, command.vertex_offset, command.first_instance);
	}

	void CommandBufferRecording::draw_indirect(VulkanCommand::DrawIndirect command)
	{
		this->register_command(command);
		vkCmdDrawIndirect(this->get_command_buffer().native(), command.draw_indirect_buffer->native(), command.offset, command.draw_count, command.stride);
	}

	void CommandBufferRecording::draw_indirect_count(VulkanCommand::DrawIndirectCount command)
	{
		this->register_command(command);
		vkCmdDrawIndirectCount(this->get_command_buffer().native(), command.draw_indirect_buffer->native(), command.offset, command.draw_indirect_buffer->native(), 0, command.max_draw_count, command.stride);
	}

	void CommandBufferRecording::draw_indexed_indirect(VulkanCommand::DrawIndexedIndirect command)
	{
		this->register_command(command);
		vkCmdDrawIndexedIndirect(this->get_command_buffer().native(), command.draw_indirect_buffer->native(), command.offset, command.draw_count, command.stride);
	}

	void CommandBufferRecording::draw_indexed_indirect_count(VulkanCommand::DrawIndexedIndirectCount command)
	{
		this->register_command(command);
		vkCmdDrawIndexedIndirectCount(this->get_command_buffer().native(), command.draw_indirect_buffer->native(), command.offset, command.draw_indirect_buffer->native(), 0, command.max_draw_count, command.stride);
	}

	void CommandBufferRecording::bind_index_buffer(VulkanCommand::BindIndexBuffer command)
	{
		this->register_command(command);
		vkCmdBindIndexBuffer(this->get_command_buffer().native(), command.index_buffer->native(), command.buffer_offset, VK_INDEX_TYPE_UINT32);
	}

	void CommandBufferRecording::bind_descriptor_sets(VulkanCommand::BindDescriptorSets command)
	{
		this->register_command(command);
		std::vector<DescriptorSet::NativeType> set_natives(command.descriptor_sets.length());
		std::transform(command.descriptor_sets.begin(), command.descriptor_sets.end(), set_natives.begin(), [](const DescriptorSet* set){return set->native();});

		vkCmdBindDescriptorSets(this->get_command_buffer().native(), static_cast<VkPipelineBindPoint>(command.context), command.pipeline_layout->native(), command.first_set_id, command.descriptor_sets.length(), set_natives.data(), 0, nullptr);
	}

	void CommandBufferRecording::buffer_copy_buffer(VulkanCommand::BufferCopyBuffer command)
	{
		tz::assert(command.src != nullptr, "BufferCopyBuffer: Source buffer was nullptr");
		tz::assert(command.dst != nullptr, "BufferCopyBuffer: Destination buffer was nullptr");
		tz::assert(command.src->get_usage().contains(BufferUsage::TransferSource), "BufferCopyBuffer: Source buffer did not contain BufferUsage::TransferSource");
		tz::assert(command.dst->get_usage().contains(BufferUsage::TransferDestination), "BufferCopyBuffer: Destination buffer did not contain BufferUsage::TransferDestination");

		this->register_command(command);
		VkBufferCopy cpy
		{
			.srcOffset = 0,
			.dstOffset = 0,
			.size = std::min(command.src->size(), command.dst->size())
		};
		vkCmdCopyBuffer(this->get_command_buffer().native(), command.src->native(), command.dst->native(), 1, &cpy);
	}

	void CommandBufferRecording::buffer_copy_image(VulkanCommand::BufferCopyImage command)
	{
		tz::assert(command.src != nullptr, "BufferCopyImage: Source buffer was nullptr");
		tz::assert(command.dst != nullptr, "BufferCopyImage: Destination image was nullptr");
		tz::assert(command.src->get_usage().contains(BufferUsage::TransferSource), "BufferCopyImage: Source buffer did not contain BufferUsage::TransferSource");

		// So ideally we could just use command.dst->get_layout(). However, it's very possible this recording contains a previous command which will have changed that layout. We will check to make sure. If so, we use the updated layout (even though it hasn't happened yet), otherwise we just use the images current layout.
		ImageLayout img_layout = this->get_layout_so_far(*command.dst);
		tz::assert(img_layout == ImageLayout::TransferDestination, "BufferCopyImage:: Destination image was not in TransferDestination ImageLayout, so it cannot be an, erm, transfer destination.");

		tz::vec2ui img_dims = command.dst->get_dimensions();
		VkBufferImageCopy cpy
		{
			.bufferOffset = 0,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageSubresource =
			{
				.aspectMask = static_cast<VkImageAspectFlags>(static_cast<ImageAspectFlag>(command.image_aspects)),
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1
			},
			.imageOffset = VkOffset3D{.x = 0, .y = 0, .z = 0},
			.imageExtent = VkExtent3D{.width = img_dims[0], .height = img_dims[1], .depth = 1}
		};
		vkCmdCopyBufferToImage(this->get_command_buffer().native(), command.src->native(), command.dst->native(), static_cast<VkImageLayout>(img_layout), 1, &cpy);
		this->register_command(command);
	}

	void CommandBufferRecording::image_copy_image(VulkanCommand::ImageCopyImage command)
	{
		tz::assert(command.src != nullptr, "ImageCopyImage contained nullptr for its source image. Please submit a bug report.");
		tz::assert(command.dst != nullptr, "ImageCopyImage contained nullptr for its destination image. Please submit a bug report.");

		tz::vec2ui min_dims;
		min_dims[0] = std::min(command.src->get_dimensions()[0], command.dst->get_dimensions()[0]);
		min_dims[1] = std::min(command.src->get_dimensions()[1], command.dst->get_dimensions()[1]);
		VkImageCopy copy_region
		{
			.srcSubresource =
			{
				.aspectMask = static_cast<VkImageAspectFlags>(static_cast<ImageAspectFlag>(command.image_aspects)),
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1
			},
			.srcOffset = {0, 0, 0},
			.dstSubresource =
			{
				.aspectMask = static_cast<VkImageAspectFlags>(static_cast<ImageAspectFlag>(command.image_aspects)),
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1
			},
			.dstOffset = {0, 0, 0},
			.extent = {min_dims[0], min_dims[1], 1u}
		};
		ImageLayout src_layout = this->get_layout_so_far(*command.src);
		ImageLayout dst_layout = this->get_layout_so_far(*command.dst);
		tz::assert(src_layout == ImageLayout::General || src_layout == ImageLayout::TransferSource, "ImageCopyImage source image was not in expected layout. It should either be TransferSource or General.");
		tz::assert(dst_layout == ImageLayout::General || src_layout == ImageLayout::TransferDestination, "ImageCopyImage destination image was not in expected layout. It should either be TransferDestination or General.");
		vkCmdCopyImage(this->get_command_buffer().native(), command.src->native(), static_cast<VkImageLayout>(src_layout), command.dst->native(), static_cast<VkImageLayout>(dst_layout), 1, &copy_region);
		this->register_command(command);
	}

	void CommandBufferRecording::bind_buffer(VulkanCommand::BindBuffer command)
	{
		tz::assert(command.buffer != nullptr, "BindBuffer contained nullptr Buffer. Please submit a bug report.");
		Buffer::NativeType buf_native = command.buffer->native();
		VkDeviceSize offsets[] = {0};
		if(command.buffer->get_usage().contains(BufferUsage::VertexBuffer))
		{
			vkCmdBindVertexBuffers(this->get_command_buffer().native(), 0, 1, &buf_native, offsets);
		}
		else if(command.buffer->get_usage().contains(BufferUsage::index_buffer))
		{
			vkCmdBindIndexBuffer(this->get_command_buffer().native(), buf_native, 0, VK_INDEX_TYPE_UINT32);
		}
		else
		{
			tz::error("BindBuffer contained a Buffer that was not a VertexBuffer nor index_buffer. Please submit a bug report.");
			return;
		}
		this->register_command(command);
	}

	void CommandBufferRecording::transition_image_layout(VulkanCommand::TransitionImageLayout command)
	{
		tz::assert(command.image != nullptr, "TransitionImageLayout contained nullptr Image");
		// Given a list of mip-levels/array layers X:
		// X.begin() -> X.end() must be sequential with no gaps.
		// i.e {-1, 0, 1, 2, 3} is fine {-1, 1, 2, 3} is not, neither is {-1, 2, 3, 0, 1}
		// Assert that neither of these are the case.
		auto is_ascending_with_no_gaps_ui32 = [](auto begin, auto end)->bool
		{
			std::vector<std::uint32_t> iota_version(std::distance(begin, end));
			std::iota(iota_version.begin(), iota_version.end(), *begin);
			return std::equal(begin, end, iota_version.begin());
		};

		tz::assert(is_ascending_with_no_gaps_ui32(command.affected_mip_levels.begin(), command.affected_mip_levels.end()), "TransitionImageLayout contained list of affected mip levels of size %zu. However, the elements were not 'sequential without gaps'.", command.affected_mip_levels.length());
		tz::assert(is_ascending_with_no_gaps_ui32(command.affected_layers.begin(), command.affected_layers.end()), "TransitionImageLayout contained list of affected array layers of size %zu. However, the elements were not 'sequential without gaps'.", command.affected_layers.length());

		VkImageSubresourceRange img_subres_range
		{
			.aspectMask = static_cast<VkImageAspectFlags>(static_cast<ImageAspectFlag>(command.image_aspects)),
			.baseMipLevel = command.affected_mip_levels.front(),
			.levelCount = static_cast<std::uint32_t>(command.affected_mip_levels.length()),
			.baseArrayLayer = command.affected_layers.front(),
			.layerCount = static_cast<std::uint32_t>(command.affected_layers.length())
		};

		ImageLayout cur_layout = this->get_layout_so_far(*command.image);
		if(command.old_layout.has_value())
		{
			cur_layout = command.old_layout.value();
		}
		this->register_command(command);
		VkImageMemoryBarrier barrier
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = static_cast<VkAccessFlags>(static_cast<AccessFlag>(command.source_access)),
			.dstAccessMask = static_cast<VkAccessFlags>(static_cast<AccessFlag>(command.destination_access)),
			.oldLayout = static_cast<VkImageLayout>(cur_layout),
			.newLayout = static_cast<VkImageLayout>(command.target_layout),
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = command.image->native(),
			.subresourceRange = img_subres_range
		};

		vkCmdPipelineBarrier(this->get_command_buffer().native(), static_cast<VkPipelineStageFlags>(command.source_stage), static_cast<VkPipelineStageFlags>(command.destination_stage), 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}

	void CommandBufferRecording::set_scissor_dynamic(VulkanCommand::SetScissorDynamic command)
	{
		this->register_command(command);
		VkRect2D rect;
		rect.offset = {static_cast<std::int32_t>(command.offset[0]), static_cast<std::int32_t>(command.offset[1])};
		rect.extent = {static_cast<std::uint32_t>(command.extent[0]), static_cast<std::uint32_t>(command.extent[1])};
		vkCmdSetScissor(this->get_command_buffer().native(), 0, 1, &rect);
	}

	void CommandBufferRecording::debug_begin_label([[maybe_unused]] VulkanCommand::DebugBeginLabel command)
	{
		#if TZ_DEBUG
			this->register_command(command);

			const VulkanInstance& inst = this->get_command_buffer().get_device().get_hardware().get_instance();
			VkDebugUtilsLabelEXT label
			{
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
				.pNext = nullptr,
				.pLabelName = command.name.c_str(),
				.color = {}
			};
			label.color[0] = command.colour[0];
			label.color[1] = command.colour[1];
			label.color[2] = command.colour[2];
			label.color[3] = command.colour[3];
			inst.ext_begin_debug_utils_label(this->get_command_buffer().native(), label);
		#endif
	}

	void CommandBufferRecording::debug_end_label([[maybe_unused]] VulkanCommand::DebugEndLabel command)
	{
		#if TZ_DEBUG
			this->register_command(command);

			const VulkanInstance& inst = this->get_command_buffer().get_device().get_hardware().get_instance();
			inst.ext_end_debug_utils_label(this->get_command_buffer().native());
		#endif
	}

	const CommandBuffer& CommandBufferRecording::get_command_buffer() const
	{
		tz::assert(this->command_buffer != nullptr, "CommandBufferRecording had nullptr CommandBuffer. Please submit a bug report");
		return *this->command_buffer;
	}

	CommandBuffer& CommandBufferRecording::get_command_buffer()
	{
		tz::assert(this->command_buffer != nullptr, "CommandBufferRecording had nullptr CommandBuffer. Please submit a bug report");
		return *this->command_buffer;
	}

	void CommandBufferRecording::register_command(VulkanCommand::variant command)
	{
		CommandBuffer& buf = this->get_command_buffer();
		tz::assert(buf.is_recording(), "CommandBufferRecording tried to register a command, but the CommandBuffer isn't actually recording. Please submit a bug report.");
		buf.add_command(command);
	}

	ImageLayout CommandBufferRecording::get_layout_so_far(const Image& image) const
	{
		ImageLayout img_layout = image.get_layout();
		for(const VulkanCommand::variant& previous_command : this->get_command_buffer().get_recorded_commands())
		{
			std::visit([&img_layout, &image](auto&& arg)
			{
				using T = std::decay_t<decltype(arg)>;
				if constexpr(std::is_same_v<T, VulkanCommand::TransitionImageLayout>)
				{
					// If the image layout will have been transitioned, then use the updated layout.
					if(arg.image == &image)
					{
						img_layout = arg.target_layout;
					}
				}
			}, previous_command);
		}
		return img_layout;
	}

	const LogicalDevice& CommandBuffer::get_device() const
	{
		return this->owner_pool->get_device();
	}

	CommandBufferRecording CommandBuffer::record()
	{
		tz::assert(!this->recording, "CommandBuffer already being recorded");
		this->recorded_commands.clear();
		return {*this};
	}

	bool CommandBuffer::is_recording() const
	{
		return this->recording;
	}

	bool CommandBuffer::has_ever_recorded() const
	{
		return this->ever_recorded;
	}

	std::size_t CommandBuffer::command_count() const
	{
		return this->recorded_commands.size();
	}

	std::span<const VulkanCommand::variant> CommandBuffer::get_recorded_commands() const
	{
		return this->recorded_commands;
	}

	void CommandBuffer::set_owner(const CommandPool& owner_pool)
	{
		this->owner_pool = &owner_pool;
	}

	CommandBuffer::NativeType CommandBuffer::native() const
	{
		return this->command_buffer;
	}

	void CommandBuffer::set_recording(bool recording)
	{
		this->recording = recording;
		this->ever_recorded = true;
	}

	void CommandBuffer::add_command(VulkanCommand::variant command)
	{
		this->recorded_commands.push_back(command);
	}
	
	CommandBuffer::CommandBuffer(const CommandPool& owner_pool, CommandBuffer::NativeType native):
	command_buffer(native),
	owner_pool(&owner_pool),
	recording(false),
	ever_recorded(false)
	{

	}

	bool CommandPool::AllocationResult::success() const
	{
		return this->type == CommandPool::AllocationResult::AllocationResultType::AllocationSuccess;
	}

	CommandPool::CommandPool(CommandPoolInfo info):
	pool(VK_NULL_HANDLE),
	info(info)
	{
		VkCommandPoolCreateInfo create
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = static_cast<VkCommandPoolCreateFlags>(static_cast<CommandPoolFlag>(info.flags)),
			.queueFamilyIndex = this->info.queue->get_info().queue_family_idx
		};

		VkResult res = vkCreateCommandPool(this->get_device().native(), &create, nullptr, &this->pool);
		switch(res)
		{
			case VK_SUCCESS:
				// do nothing
			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				tz::error("Failed to create CommandPool because we ran out of host memory (RAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				tz::error("Failed to create CommandPool because we ran out of device memory (VRAM). Please ensure that your system meets the minimum requirements.");
			break;
			default:
				tz::error("Failed to create CommandPool but cannot determine why. Please submit a bug report.");
			break;
		}
	}

	CommandPool::CommandPool(CommandPool&& move):
	pool(VK_NULL_HANDLE),
	info()
	{
		*this = std::move(move);
	}

	CommandPool::~CommandPool()
	{
		if(this->pool != VK_NULL_HANDLE)
		{
			vkDestroyCommandPool(this->get_device().native(), this->pool, nullptr);
			this->pool = VK_NULL_HANDLE;
		}
	}

	CommandPool& CommandPool::operator=(CommandPool&& rhs)
	{
		std::swap(this->pool, rhs.pool);
		std::swap(this->info, rhs.info);
		return *this;
	}

	const LogicalDevice& CommandPool::get_device() const
	{
		return this->info.queue->get_device();
	}

	CommandPool::AllocationResult CommandPool::allocate_buffers(const Allocation& alloc)
	{
		VkCommandBufferAllocateInfo create
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = nullptr,
			.commandPool = this->pool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = alloc.buffer_count
		};

		std::vector<CommandBuffer::NativeType> cmd_buf_natives(alloc.buffer_count);

		VkResult res = vkAllocateCommandBuffers(this->get_device().native(), &create, cmd_buf_natives.data());
		CommandPool::AllocationResult alloc_res;
		switch(res)
		{
			case VK_SUCCESS:
				alloc_res.type = CommandPool::AllocationResult::AllocationResultType::AllocationSuccess;
				for(CommandBuffer::NativeType cmd_buf_native : cmd_buf_natives)
				{
					alloc_res.buffers.add({*this, cmd_buf_native});
				}
			break;
			default:
				alloc_res.type = CommandPool::AllocationResult::AllocationResultType::FatalError;
			break;
		}

		return alloc_res;
	}

	void CommandPool::free_buffers(const AllocationResult& alloc_result)
	{
		std::vector<CommandBuffer::NativeType> cmd_natives(alloc_result.buffers.length());
		std::transform(alloc_result.buffers.begin(), alloc_result.buffers.end(), cmd_natives.begin(),
		[](const vk2::CommandBuffer& buf)
		{
			return buf.native();
		});
		vkFreeCommandBuffers(this->get_device().native(), this->pool, cmd_natives.size(), cmd_natives.data());
	}

	CommandPool::NativeType CommandPool::native() const
	{
		return this->pool;
	}

	CommandPool CommandPool::null()
	{
		return {};
	}

	bool CommandPool::is_null() const
	{
		return this->pool == VK_NULL_HANDLE;
	}

	CommandPool::CommandPool():
	pool(VK_NULL_HANDLE),
	info()
	{

	}

}

#endif // TZ_VULKAN
