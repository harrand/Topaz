#if TZ_VULKAN
#include "core/assert.hpp"
#include "gl/impl/frontend/vk/shader.hpp"
#include <fstream>

namespace tz::gl
{
	ShaderVulkan::ShaderVulkan(const vk::LogicalDevice& device, ShaderBuilderVulkan builder)
	{
		std::string all_metadata;
		std::string_view compute_source = builder.get_shader_source(ShaderType::ComputeShader);
		if(!compute_source.empty())
		{
			// We have a compute shader!
			this->compute_shader = {device, {compute_source.data(), compute_source.size()}};
			all_metadata += builder.get_shader_meta(ShaderType::ComputeShader);
		}
		else
		{
			// No compute shader. We *must* have a valid vertex and fragment shader
			std::string_view vertex_source = builder.get_shader_source(ShaderType::VertexShader);
			std::string_view fragment_source = builder.get_shader_source(ShaderType::FragmentShader);
			tz_assert(!vertex_source.empty(), "Vertex shader source must be present if there is no compute shader");
			tz_assert(!fragment_source.empty(), "Fragment shader source must be present if there is no compute shader");
			this->vertex_shader = {device, {vertex_source.data(), vertex_source.size()}};
			this->fragment_shader = {device, {fragment_source.data(), fragment_source.size()}};
			all_metadata += std::string("\n") + static_cast<std::string>(builder.get_shader_meta(ShaderType::VertexShader));
			all_metadata += std::string("\n") + static_cast<std::string>(builder.get_shader_meta(ShaderType::FragmentShader));
		}
		this->meta = ShaderMeta::from_metadata_string(all_metadata);
	}

	const vk::ShaderModule& ShaderVulkan::vk_get_vertex_shader() const
	{
		return this->vertex_shader.value();
	}
	
	const vk::ShaderModule& ShaderVulkan::vk_get_fragment_shader() const
	{
		return this->fragment_shader.value();
	}

	const vk::ShaderModule& ShaderVulkan::vk_get_compute_shader() const
	{
		return this->compute_shader.value();
	}

	const ShaderMeta& ShaderVulkan::get_meta() const
	{
		return this->meta;
	}

}

#endif // TZ_VULKAN