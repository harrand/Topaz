namespace tz::gl2
{
	void CommandProcessor::do_scratch_operations(tz::Action<vk2::CommandBufferRecording&> auto record_commands)
	{
		vk2::CommandBuffer scratch_buf = this->commands.buffers.back();
		{
			vk2::CommandBufferRecording record = scratch_buf.record();
			record_commands(record);
		}
		vk2::Fence work_complete_fence
		{{
			.device = &this->command_pool.get_device()
		}};
		this->graphics_queue->submit
		({
			.command_buffers = {&scratch_buf},
			.waits = {},
			.signal_semaphores = {},
			.execution_complete_fence = &work_complete_fence
		});
		work_complete_fence.wait_until_signalled();
	}
	void CommandProcessor::set_rendering_commands(tz::Action<vk2::CommandBufferRecording&, std::size_t> auto record_commands)
	{
		for(std::size_t i = 0; i < this->get_render_command_buffers().size(); i++)
		{
			vk2::CommandBufferRecording record = this->get_render_command_buffers()[i].record();
			record_commands(record, i);
		}
	}
}
