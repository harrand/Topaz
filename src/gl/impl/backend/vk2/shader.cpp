#if TZ_VULKAN
#include "gl/impl/backend/vk2/shader.hpp"

namespace tz::gl::vk2
{
	ShaderModule::ShaderModule(const ShaderModuleInfo& info):
	device(info.device),
	type(info.type),
	shader_module(VK_NULL_HANDLE)
	{
		tz_assert(this->device != nullptr && !this->device->is_null(), "ShaderModuleInfo has nullptr or null LogicalDevice");

		VkShaderModuleCreateInfo create{};
		create.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		create.codeSize = info.code.size();
		create.pCode = reinterpret_cast<const std::uint32_t*>(info.code.data());

		VkResult res = vkCreateShaderModule(this->device->native(), &create, nullptr, &this->shader_module);
		switch(res)
		{
			case VK_SUCCESS:

			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				tz_error("Ran out of host memory (RAM) while trying to create ShaderModule. Ensure your system meets the minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				tz_error("Ran out of device memory (VRAM) while trying to create ShaderModule. Ensure your system meets the minimum requirements.");
			break;
			case VK_ERROR_INVALID_SHADER_NV:
				tz_error("Shader Module failed to compile/link. Please submit a bug report.");
			break;
			default:
				tz_error("Failed to create ShaderModule but don't know why. This return code is undocumented. Please submit a bug report");
			break;
		}
	}

	ShaderModule::ShaderModule(ShaderModule&& move):
	device(nullptr),
	type(),
	shader_module(VK_NULL_HANDLE)
	{
		*this = std::move(move);
	}

	ShaderModule::~ShaderModule()
	{
		if(this->shader_module != VK_NULL_HANDLE)
		{
			tz_assert(this->device != nullptr && !this->device->is_null(), "Can't destroy ShaderModule because LogicalDevice is nullptr or null");
			vkDestroyShaderModule(this->device->native(), this->shader_module, nullptr);
			this->shader_module = VK_NULL_HANDLE;
		}
	}

	ShaderModule& ShaderModule::operator=(ShaderModule&& rhs)
	{
		std::swap(this->device, rhs.device);
		std::swap(this->type, rhs.type);
		std::swap(this->shader_module, rhs.shader_module);
		return *this;
	}	

}

#endif // TZ_VULKAN
