#if TZ_OGL
#include "gl/impl/frontend/ogl2/renderer.hpp"
#include "gl/impl/frontend/ogl2/component.hpp"

namespace tz::gl2
{
	ResourceStorage::ResourceStorage(std::span<const IResource* const> resources):
	AssetStorageCommon<IResource>(resources),
	components()
	{
		for(std::size_t i = 0; i < this->count(); i++)
		{
			IResource* res = this->get(static_cast<tz::HandleValue>(i));
			switch(res->get_type())
			{
				case ResourceType::Buffer:
					this->components.push_back(std::make_unique<BufferComponentOGL>(*res));
					if(res->get_access() == ResourceAccess::DynamicFixed)
					{
						// Tell the resource to use the buffer's data. Also copy whatever we had before.
						std::span<const std::byte> initial_data = res->data();
						std::span<std::byte> buffer_byte_data = static_cast<BufferComponentOGL*>(this->components.back().get())->ogl_get_buffer().map_as<std::byte>();
						std::copy(initial_data.begin(), initial_data.end(), buffer_byte_data.begin());
						res->set_mapped_data(buffer_byte_data);
					}
				break;
				case ResourceType::Image:
					this->components.push_back(std::make_unique<ImageComponentOGL>(*res));
				break;
				default:
					tz_error("Unrecognised ResourceType. Please submit a bug report.");
				break;
			}
		}
	}

	const IComponent* ResourceStorage::get_component(ResourceHandle handle) const
	{
		return this->components[static_cast<std::size_t>(static_cast<tz::HandleValue>(handle))].get();
	}

	IComponent* ResourceStorage::get_component(ResourceHandle handle)
	{
		return this->components[static_cast<std::size_t>(static_cast<tz::HandleValue>(handle))].get();
	}

//--------------------------------------------------------------------------------------------------

	ShaderManager::ShaderManager(const ShaderInfo& sinfo):
	shader(this->make_shader(sinfo))
	{

	}

	void ShaderManager::use()
	{
		this->shader.use();
	}

	ogl2::Shader ShaderManager::make_shader(const ShaderInfo& sinfo) const
	{
		tz::BasicList<ogl2::ShaderModuleInfo> modules;
		if(sinfo.has_shader(ShaderStage::Compute))
		{
			// Compute, we only care about the compute shader.
			modules =
			{
				{
					.type = ogl2::ShaderType::Compute,
					.code = ogl2::OGLString(sinfo.get_shader(ShaderStage::Compute))
				}
			};
		}
		else
		{
			// Graphics, must contain a Vertex and Fragment shader.
			tz_assert(sinfo.has_shader(ShaderStage::Vertex), "ShaderInfo must contain a non-empty vertex shader if no compute shader is present.");
			tz_assert(sinfo.has_shader(ShaderStage::Fragment), "ShaderInfo must contain a non-empty fragment shader if no compute shader is present.");
			modules =
			{
				{
					.type = ogl2::ShaderType::Vertex,
					.code = ogl2::OGLString(sinfo.get_shader(ShaderStage::Vertex))
				},
				{
					.type = ogl2::ShaderType::Fragment,
					.code = ogl2::OGLString(sinfo.get_shader(ShaderStage::Fragment))
				}
			};
		}
		return
		{{
			.modules = modules
		}};
	}

//--------------------------------------------------------------------------------------------------
	
	RendererOGL::RendererOGL(const RendererInfoOGL& info):
	vao(),
	resources(info.get_resources()),
	shader(info.shader()),
	options(info.get_options())
	{

	}

	unsigned int RendererOGL::resource_count() const
	{
		return this->resources.count();
	}

	const IResource* RendererOGL::get_resource(ResourceHandle handle) const
	{
		return this->resources.get(handle);
	}

	IResource* RendererOGL::get_resource(ResourceHandle handle)
	{
		return this->resources.get(handle);
	}

	const IComponent* RendererOGL::get_component(ResourceHandle handle) const
	{
		return this->resources.get_component(handle);
	}

	IComponent* RendererOGL::get_component(ResourceHandle handle)
	{
		return this->resources.get_component(handle);
	}

	const RendererOptions& RendererOGL::get_options() const
	{
		return this->options;
	}

	void RendererOGL::render()
	{
		this->vao.draw(this->tri_count);
	}

	void RendererOGL::render(unsigned int tri_count)
	{
		this->tri_count = tri_count;
		this->render();
	}

}

#endif // TZ_OGL
