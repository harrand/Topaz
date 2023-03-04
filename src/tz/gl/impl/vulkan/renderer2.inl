namespace tz::gl
{
	void renderer_command_processor::do_scratch_work(tz::action<vk2::CommandBufferRecording&> auto record_commands)
	{
		// firstly record the commands requested.
		{
		vk2::CommandBufferRecording rec = this->scratch_command_buffer().record();
		record_commands(rec);
		}
		// then, execute them.
		tz::error("NYI");
	}
}
