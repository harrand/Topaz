#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_SHADER_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_SHADER_HPP  
#if TZ_VULKAN
#include "tz/core/data/enum_field.hpp"
#include "tz/gl/impl/vulkan/detail/logical_device.hpp"

namespace tz::gl::vk2
{
	/**
	 * @ingroup tz_gl_vk_graphics_pipeline_shader
	 * Supported Shader Type
	 */
	enum class ShaderType
	{
		vertex = VK_SHADER_STAGE_VERTEX_BIT,
		tessellation_control = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
		tessellation_evaluation = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
		fragment = VK_SHADER_STAGE_FRAGMENT_BIT,

		compute = VK_SHADER_STAGE_COMPUTE_BIT
	};
	
	using ShaderTypeField = tz::enum_field<ShaderType>;

	/**
	 * @ingroup tz_gl_vk_graphics_pipeline_shader
	 * Specifies parameters of a single ShaderModule.
	 */
	struct ShaderModuleInfo
	{
		/// LogicalDevice owning the ShaderModule. Must not be null.
		const LogicalDevice* device;
		/// Type of the shader.
		ShaderType type;
		/// COntainer for valid SPIRV code.
		std::vector<char> code;
	};

	/**
	 * @ingroup tz_gl_vk_graphics_pipeline_shader
	 * Represents a single module of a Shader. Shaders are comprised of one or more ShaderModules of a given type. ShaderModules are not executable on their own.
	 */
	class ShaderModule
	{
	public:
		/**
		 * Create a ShaderModule based upon some existing SPIRV code.
		 */
		ShaderModule(const ShaderModuleInfo& info);
		ShaderModule(const ShaderModule& copy) = delete;
		ShaderModule(ShaderModule&& move);
		~ShaderModule();

		ShaderModule& operator=(const ShaderModule& rhs) = delete;
		ShaderModule& operator=(ShaderModule&& rhs);

		/**
		 * Retrieve the type of this shader module.
		 */
		ShaderType get_type() const;

		const LogicalDevice& get_device() const;

		using NativeType = VkShaderModule;
		NativeType native() const;
	private:
		const LogicalDevice* device;
		ShaderType type;
		VkShaderModule shader_module;
	};

	/**
	 * @ingroup tz_gl_vk_graphics_pipeline_shader
	 * Specifies parameters of a Shader, and all the modules that comprise it.
	 */
	struct ShaderInfo
	{
		/// LogicalDevice owner. Must not be null.
		const LogicalDevice* device;
		/// Information about the Shader's modules. Must be a valid combination of modules.
		tz::basic_list<ShaderModuleInfo> modules;
	};

	struct ShaderPipelineData
	{
		tz::basic_list<VkPipelineShaderStageCreateInfo> create_infos;
	};

	/**
	 * @ingroup tz_gl_vk_graphics_pipeline_shader
	 * Represents a Shader program. Can be used within a @ref GraphicsPipeline or @ref ComputePipeline.
	 */
	class Shader
	{
	public:
		Shader(const ShaderInfo& info);
		Shader(const Shader& copy) = delete;
		Shader(Shader&& move);
		~Shader() = default;

		Shader& operator=(const Shader& rhs) = delete;
		Shader& operator=(Shader&& rhs);

		ShaderPipelineData native_data() const;
		bool is_compute() const;

		std::string debug_get_name() const;
		void debug_set_name(std::string debug_name);

		static Shader null();
		bool is_null() const;
	private:
		Shader();

		std::vector<ShaderModule> modules;
		std::string debug_name = "";
	};
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_SHADER_HPP
