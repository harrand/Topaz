#include "gl/impl/backend/ogl2/buffer.hpp"
#if TZ_OGL
#include "gl/impl/frontend/ogl2/renderer.hpp"
#include "gl/impl/frontend/ogl2/component.hpp"

namespace tz::gl2
{
	ResourceStorage::ResourceStorage(std::span<const IResource* const> resources):
	AssetStorageCommon<IResource>(resources),
	components(),
	image_handles(),
	bindless_image_storage_buffer(ogl2::Buffer::null())
	{
		for(std::size_t i = 0; i < this->count(); i++)
		{
			IResource* res = this->get(static_cast<tz::HandleValue>(i));
			switch(res->get_type())
			{
				case ResourceType::Buffer:
					this->components.push_back(std::make_unique<BufferComponentOGL>(*res));
					{
						BufferComponentOGL& buf = *static_cast<BufferComponentOGL*>(this->components.back().get());
						ogl2::Buffer& buffer = buf.ogl_get_buffer();
						switch(buf.get_resource()->get_access())
						{
							case ResourceAccess::StaticFixed:
								{
									// Create staging buffer and do a copy.
									ogl2::Buffer staging
									{{
										.target = ogl2::BufferTarget::ShaderStorage,
										.residency = ogl2::BufferResidency::Dynamic,
										.size_bytes = buffer.size()
									}};
									std::span<std::byte> staging_data = staging.map_as<std::byte>();
									std::span<const std::byte> resource_data = res->data();
									std::copy(resource_data.begin(), resource_data.end(), staging_data.begin());
									ogl2::buffer::copy(staging, buffer);
								}
							break;
							case ResourceAccess::DynamicFixed:
							[[fallthrough]];
							case ResourceAccess::DynamicVariable:
							{

								// Tell the resource to use the buffer's data. Also copy whatever we had before.
								std::span<const std::byte> initial_data = res->data();
								std::span<std::byte> buffer_byte_data = buffer.map_as<std::byte>();
								std::copy(initial_data.begin(), initial_data.end(), buffer_byte_data.begin());
								res->set_mapped_data(buffer_byte_data);
							}
							break;
							default:
								tz_error("ResourceAccess for this buffer is not yet implemented.");
							break;
						}
					}
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
					{
						ImageComponentOGL& img = *static_cast<ImageComponentOGL*>(this->components.back().get());
						ogl2::Image& image = img.ogl_get_image();
						image.set_data(img.get_resource()->data());
						image.make_bindless();
						this->image_handles.push_back(img.ogl_get_image().get_bindless_handle());
					}
				break;
				default:
					tz_error("Unrecognised ResourceType. Please submit a bug report.");
				break;
			}
		}
		this->fill_bindless_image_buffer();
	}

	const IComponent* ResourceStorage::get_component(ResourceHandle handle) const
	{
		return this->components[static_cast<std::size_t>(static_cast<tz::HandleValue>(handle))].get();
	}

	IComponent* ResourceStorage::get_component(ResourceHandle handle)
	{
		return this->components[static_cast<std::size_t>(static_cast<tz::HandleValue>(handle))].get();
	}

	unsigned int ResourceStorage::resource_count_of(ResourceType type) const
	{
		return std::count_if(this->components.begin(), this->components.end(),
		[type](const auto& component_ptr)
		{
			return component_ptr->get_resource()->get_type() == type;
		});
	}

	void ResourceStorage::fill_bindless_image_buffer()
	{
		if(this->image_handles.empty())
		{
			return;
		}
		const std::size_t buf_size = this->image_handles.size() * sizeof(ogl2::Image::BindlessTextureHandle);
		this->bindless_image_storage_buffer =
		{{
			.target = ogl2::BufferTarget::ShaderStorage,
			.residency = ogl2::BufferResidency::Static,
			.size_bytes = buf_size
		}};
		ogl2::Buffer temp_copy_buffer =
		{{
			.target = ogl2::BufferTarget::ShaderStorage,
			.residency = ogl2::BufferResidency::Dynamic,
			.size_bytes = buf_size
		}};
		{
			std::span<ogl2::Image::BindlessTextureHandle> handle_data = temp_copy_buffer.map_as<ogl2::Image::BindlessTextureHandle>();
			std::copy(this->image_handles.begin(), this->image_handles.end(), handle_data.begin());
		}
		ogl2::buffer::copy(temp_copy_buffer, this->bindless_image_storage_buffer);
	}

	void ResourceStorage::bind_buffers()
	{
		if(this->resource_count_of(ResourceType::Buffer) == 0)
		{
			return;
		}
		std::size_t i = 0;
		for(std::size_t j = 0; j < this->components.size(); j++)
		{
			IComponent* comp = this->components[j].get();
			if(comp->get_resource()->get_type() == ResourceType::Buffer)
			{
				static_cast<BufferComponentOGL*>(comp)->ogl_get_buffer().bind_to_resource_id(i++);
			}
		}
		tz_assert(i == this->resource_count_of(ResourceType::Buffer), "ResourceStorage::bind_buffers had some kind of logic error and didn't find all the buffers. Please submit a bug report.");
	}

	void ResourceStorage::bind_image_buffer()
	{
		if(!this->bindless_image_storage_buffer.is_null())
		{
			this->bindless_image_storage_buffer.bind_to_resource_id(this->resource_count_of(ResourceType::Buffer));
		}
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

	OutputManager::OutputManager(IOutput* output):
	output(output),
	framebuffer(ogl2::Framebuffer::null())
	{
		if(this->output != nullptr)
		{
			switch(this->output->get_target())
			{
			case OutputTarget::OffscreenImage:

			break;
			case OutputTarget::Window:
				// Do nothing. We use null framebuffer.
			break;
			default:
				tz_error("Use of this specific OutputTarget is not yet implemented.");
			break;
			}
		}
	}

	void OutputManager::set_render_target() const
	{
		this->framebuffer.bind();
		this->framebuffer.clear();
	}

//--------------------------------------------------------------------------------------------------
	
	RendererOGL::RendererOGL(RendererInfoOGL info):
	vao(),
	resources(info.get_resources()),
	shader(info.shader()),
	output(info.get_output()),
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
		if(this->options.contains(RendererOption::NoDepthTesting))
		{
			glDisable(GL_DEPTH_TEST);
		}
		else
		{
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
		}
		this->output.set_render_target();

		this->shader.use();
		this->resources.bind_buffers();
		if(this->resources.resource_count_of(ResourceType::Image) > 0)
		{
			this->resources.bind_image_buffer();
		}
		this->vao.draw(this->tri_count);
	}

	void RendererOGL::render(unsigned int tri_count)
	{
		this->tri_count = tri_count;
		this->render();
	}

	void RendererOGL::edit(const RendererEditRequest& edit_request)
	{
		if(edit_request.component_edits.empty())
		{
			return;
		}
		for(const RendererComponentEditRequest& component_edit : edit_request.component_edits)
		{

			std::visit(
			[this](auto&& arg)
			{
				using T = std::decay_t<decltype(arg)>;
				if constexpr(std::is_same_v<T, RendererBufferComponentEditRequest>)
				{
					auto buf_comp = static_cast<BufferComponentOGL*>(this->get_component(arg.buffer_handle));
					tz_assert(buf_comp->get_resource()->get_access() == ResourceAccess::DynamicVariable, "Detected attempted resize of buffer resource (id %zu), but it ResourceAccess is not DynamicVariable. This means it is a fixed-size resource, so attempting to resize it is invalid.", static_cast<std::size_t>(static_cast<tz::HandleValue>(arg.buffer_handle)));
					// Make new buffer copy, and swap them with the component's held buffer. That is literally it I believe.
					ogl2::Buffer& old_buffer = buf_comp->ogl_get_buffer();
					old_buffer = ogl2::buffer::clone_resized(old_buffer, arg.size);
					// If we were dynamic, the resource mapping needs to refer to the new buffer though.
					if(buf_comp->get_resource()->get_access() == ResourceAccess::DynamicVariable)
					{
						buf_comp->get_resource()->set_mapped_data(old_buffer.map_as<std::byte>());
					}
				}
				else if constexpr(std::is_same_v<T, RendererImageComponentEditRequest>)
				{
					tz_error("Resizing images is not yet implemented");
				}
				else
				{
					tz_error("Unsupported variant type");
				}
			}, component_edit);
		}
	}
}

#endif // TZ_OGL
