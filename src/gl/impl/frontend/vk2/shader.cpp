#include "gl/impl/backend/vk2/shader.hpp"
#if TZ_VULKAN
#include "gl/impl/frontend/vk2/shader.hpp"

namespace tz::gl
{
	ShaderVulkan2::ShaderVulkan2(ShaderBuilderVulkan2 builder, ShaderDeviceInfoVulkan2 device_info):
	shader(vk2::Shader::null()),
	meta()
	{
		std::string all_metadata;
		tz::BasicList<vk2::ShaderModuleInfo> modules;

		if(builder.has_shader(ShaderType::ComputeShader))
		{
			std::string_view shader_bin = builder.get_shader_source(ShaderType::ComputeShader);
			all_metadata += shader_bin;
			std::vector<char> binary(shader_bin.length());
			std::copy(shader_bin.begin(), shader_bin.end(), binary.begin());

			modules.add
			(vk2::ShaderModuleInfo{
				.device = device_info.device,
				.type = vk2::ShaderType::Compute,
				.code = std::move(binary)
			});
		}
		else
		{
			// We must have a fragment shader.
			tz_assert(builder.has_shader(ShaderType::FragmentShader), "Shader contains neither Compute nor Fragment shader, one of these is required.");
			{
				std::string_view fragment_bin = builder.get_shader_source(ShaderType::FragmentShader);
				all_metadata += fragment_bin;
				std::vector<char> binary(fragment_bin.length());
				std::copy(fragment_bin.begin(), fragment_bin.end(), binary.begin());
				
				modules.add
				(vk2::ShaderModuleInfo{
					.device = device_info.device,
					.type = vk2::ShaderType::Fragment,
					.code = std::move(binary)
				});
			}
			if(builder.has_shader(ShaderType::VertexShader))
			{
				std::string_view vertex_bin = builder.get_shader_source(ShaderType::VertexShader);
				all_metadata += vertex_bin;
				std::vector<char> binary(vertex_bin.length());
				std::copy(vertex_bin.begin(), vertex_bin.end(), binary.begin());
				
				modules.add
				(vk2::ShaderModuleInfo{
					.device = device_info.device,
					.type = vk2::ShaderType::Vertex,
					.code = std::move(binary)
				});
			}

			this->shader =
			{{
				.device = device_info.device,
				.modules = modules
			}};
		}
		
		this->meta = ShaderMeta::from_metadata_string(all_metadata);
	}

	const ShaderMeta& ShaderVulkan2::get_meta() const
	{
		return this->meta;
	}

	const vk2::Shader& ShaderVulkan2::vk_get_shader() const
	{
		return this->shader;
	}
}

#endif // TZ_VULKAN
