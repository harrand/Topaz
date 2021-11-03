#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_SHADER_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_SHADER_HPP  
#if TZ_VULKAN
#include "core/containers/enum_field.hpp"
#include "gl/impl/backend/vk2/logical_device.hpp"

namespace tz::gl::vk2
{
	/**
	 * @ingroup tz_gl_vk_graphics_pipeline_shader
	 * Supported Shader Type
	 */
	enum class ShaderType
	{
		Vertex,
		Fragment,

		Compute
	};

	using ShaderTypeField = tz::EnumField<ShaderType>;

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

		VkShaderModule native() const;
	private:
		const LogicalDevice* device;
		ShaderType type;
		VkShaderModule shader_module;
	};

}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_SHADER_HPP
