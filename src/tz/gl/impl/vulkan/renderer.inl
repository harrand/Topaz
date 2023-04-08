namespace tz::gl
{
	void CommandProcessor::do_scratch_operations(tz::action<vk2::CommandBufferRecording&> auto record_commands)
	{
		vk2::CommandBuffer& scratch_buf = this->commands.data.buffers.back();
		{
			vk2::CommandBufferRecording record = scratch_buf.record();
			record_commands(record);
		}
		vk2::Fence work_complete_fence
		{{
			.device = &this->commands.pool.get_device()
		}};
		this->graphics_queue->submit
		({
			.command_buffers = {&scratch_buf},
			.waits = {},
			.signals = {},
			.execution_complete_fence = &work_complete_fence
		});
		work_complete_fence.wait_until_signalled();
	}

	void CommandProcessor::set_rendering_commands(tz::action<vk2::CommandBufferRecording&, std::size_t> auto record_commands)
	{
		for(std::size_t i = 0; i < this->get_render_command_buffers().size(); i++)
		{
			vk2::CommandBufferRecording record = this->get_render_command_buffers()[i].record();
			record_commands(record, i);
		}
	}
}
