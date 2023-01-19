namespace tz::gl::vk2
{
	template<VkObjectType T>
	DebugNameable<T>::DebugNameable(const vk2::LogicalDevice& device, std::uint64_t handle):
	device(&device),
	handle(handle){}

	template<VkObjectType T>
	DebugNameable<T>::DebugNameable():
	device(nullptr),
	handle(0){}

	template<VkObjectType T>
	std::string DebugNameable<T>::debug_get_name() const
	{
		return this->debug_name;
	}
	
	template<VkObjectType T>
	void DebugNameable<T>::debug_set_name([[maybe_unused]] std::string debug_name)
	{
		#if TZ_DEBUG
			this->debug_name = debug_name;
			tz::assert(this->device != nullptr, "Attempted to set debug name for a vulkan object, but the device was nullptr. Is the object a null object?");
			VkDebugUtilsObjectNameInfoEXT info
			{
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
				.pNext = nullptr,
				.objectType = T,
				.objectHandle = this->handle,
				.pObjectName = this->debug_name.c_str()
			};
			VkResult res = this->device->get_hardware().get_instance().ext_set_debug_utils_object_name(this->device->native(), info);
			switch(res)
			{
				case VK_SUCCESS:
				break;
				default:
					tz::error("Failed to set debug name for image backend, but for unknown reason. Please submit a bug report.");
				break;
			}

		#endif
	}

	template<VkObjectType T>
	void DebugNameable<T>::debug_set_handle(std::uint64_t handle)
	{
		this->handle = handle;
		this->debug_set_name(this->debug_name);
	}

	template<VkObjectType T>
	void DebugNameable<T>::debugname_swap(DebugNameable<T>& rhs)
	{
		std::swap(this->device, rhs.device);
		std::swap(this->handle, rhs.handle);
		std::swap(this->debug_name, rhs.debug_name);
	}

}
