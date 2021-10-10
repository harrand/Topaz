#if TZ_VULKAN

namespace tz::gl::vk
{
	void FrameAdmin::set_regeneration_function(tz::Action auto regeneration_function)
	{
		this->regenerate_function = regeneration_function;
	}

}

#endif // TZ_VULKAN