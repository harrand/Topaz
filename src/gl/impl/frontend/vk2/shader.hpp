#ifndef TOPAZ_GL_IMPL_FRONTEND_VK2_SHADER_HPP
#define TOPAZ_GL_IMPL_FRONTEND_VK2_SHADER_HPP
#if TZ_VULKAN
#include "gl/impl/frontend/common/shader.hpp"
#include "gl/impl/backend/vk2/shader.hpp"

namespace tz::gl
{
	using ShaderBuilderVulkan2 = ShaderBuilderBase;

	struct ShaderDeviceInfoVulkan2
	{
		const vk2::LogicalDevice* device;
	};

	class ShaderVulkan2 : public IShader
	{
	public:
		ShaderVulkan2(ShaderBuilderVulkan2 builder, ShaderDeviceInfoVulkan2 device_info);
		virtual const ShaderMeta& get_meta() const;
		const vk2::Shader& vk_get_shader() const;
	private:
		vk2::Shader shader;
		ShaderMeta meta;
	};
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_FRONTEND_VK2_SHADER_HPP
