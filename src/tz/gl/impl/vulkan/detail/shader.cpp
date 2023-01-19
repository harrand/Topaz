#if TZ_VULKAN
#include "hdk/profile.hpp"
#include "tz/gl/impl/vulkan/detail/shader.hpp"

namespace tz::gl::vk2
{
	ShaderModule::ShaderModule(const ShaderModuleInfo& info):
	device(info.device),
	type(info.type),
	shader_module(VK_NULL_HANDLE)
	{
		HDK_PROFZONE("Vulkan Backend - ShaderModule Create (Compile)", 0xFFAA0000);
		hdk::assert(this->device != nullptr && !this->device->is_null(), "ShaderModuleInfo has nullptr or null LogicalDevice");

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
				hdk::error("Ran out of host memory (RAM) while trying to create ShaderModule. Ensure your system meets the minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				hdk::error("Ran out of device memory (VRAM) while trying to create ShaderModule. Ensure your system meets the minimum requirements.");
			break;
			case VK_ERROR_INVALID_SHADER_NV:
				hdk::error("Shader Module failed to compile/link. Please submit a bug report.");
			break;
			default:
				hdk::error("Failed to create ShaderModule but don't know why. This return code is undocumented. Please submit a bug report");
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
			hdk::assert(this->device != nullptr && !this->device->is_null(), "Can't destroy ShaderModule because LogicalDevice is nullptr or null");
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

	ShaderType ShaderModule::get_type() const
	{
		return this->type;
	}

	const LogicalDevice& ShaderModule::get_device() const
	{
		return *this->device;
	}

	ShaderModule::NativeType ShaderModule::native() const
	{
		return this->shader_module;
	}
	
	Shader::Shader(const ShaderInfo& info):
	modules()
	{
		for(const ShaderModuleInfo& module_info : info.modules)
		{
			hdk::assert(module_info.device == info.device, "ShaderInfo LogicalDevice did not match the devices specified in one or more of its ShaderModuleInfos. Please submit a bug report.");
			this->modules.emplace_back(module_info);
		}
	}

	Shader::Shader(Shader&& move):
	modules()
	{
		*this = std::move(move);
	}

	Shader& Shader::operator=(Shader&& rhs)
	{
		std::swap(this->modules, rhs.modules);
		return *this;
	}

	ShaderPipelineData Shader::native_data() const
	{
		ShaderPipelineData ret;
		for(const ShaderModule& shader_module : this->modules)
		{
			ret.create_infos.add({});
			VkPipelineShaderStageCreateInfo& cur_create = ret.create_infos.back();
			cur_create.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			cur_create.stage = static_cast<VkShaderStageFlagBits>(shader_module.get_type());
			cur_create.module = shader_module.native();
			cur_create.pName = "main";
		}
		return ret;
	}

	bool Shader::is_compute() const
	{
		return std::any_of(this->modules.begin(), this->modules.end(),
		[](const ShaderModule& mod)
		{
			return mod.get_type() == ShaderType::compute;
		});
	}

	std::string Shader::debug_get_name() const
	{
		return this->debug_name;
	}

	void Shader::debug_set_name(std::string debug_name)
	{
		this->debug_name = debug_name;
		#if HDK_DEBUG
			for(const ShaderModule& s_module : this->modules)
			{
				const char* ext;
				switch(s_module.get_type())
				{
					case ShaderType::vertex:
						ext = "vertex";
					break;
					case ShaderType::tessellation_control:
						ext = "tesscon";
					break;
					case ShaderType::tessellation_evaluation:
						ext = "tesseval";
					break;
					case ShaderType::fragment:
						ext = "fragment";
					break;
					case ShaderType::compute:
						ext = "compute";
					break;
					default:
						ext = "invalid-shader-type";
					break;
				}
				std::string name = this->debug_name + ".Shader." + ext;
				VkDebugUtilsObjectNameInfoEXT info
				{
					.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
					.pNext = nullptr,
					.objectType = VK_OBJECT_TYPE_SHADER_MODULE,
					.objectHandle = reinterpret_cast<std::uint64_t>(s_module.native()),
					.pObjectName = name.c_str()
				};

				const VulkanInstance& inst = s_module.get_device().get_hardware().get_instance();
				VkResult res = inst.ext_set_debug_utils_object_name(s_module.get_device().native(), info);
				switch(res)
				{
					case VK_SUCCESS:
					break;
					default:
						hdk::error("Failed to set debug name for image backend, but for unknown reason. Please submit a bug report.");
					break;
				}
			}
		#endif
	}

	Shader Shader::null()
	{
		return{};
	}

	bool Shader::is_null() const
	{
		return this->modules.empty();
	}

	Shader::Shader():
	modules(){}
}

#endif // TZ_VULKAN
