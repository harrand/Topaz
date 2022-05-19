#if TZ_OGL
#include "core/profiling/zone.hpp"
#include "gl/impl/backend/ogl2/tz_opengl.hpp"
#include "gl/impl/backend/ogl2/buffer.hpp"
#include "gl/impl/frontend/ogl2/renderer.hpp"
#include "gl/impl/frontend/ogl2/component.hpp"
#include "gl/output.hpp"

namespace tz::gl
{
	ResourceStorage::ResourceStorage(std::span<const IResource* const> resources, std::span<const IComponent* const> components):
	AssetStorageCommon<IResource>(resources),
	components(),
	image_handles(),
	bindless_image_storage_buffer(ogl2::Buffer::null())
	{
		TZ_PROFZONE("OpenGL Frontend - RendererOGL ResourceStorage Create", TZ_PROFCOL_RED);
		auto do_metadata = [this](IComponent* comp)
		{
			IResource* res = comp->get_resource();
			switch(res->get_type())
			{
				case ResourceType::Buffer:
					{
						BufferComponentOGL& buf = *static_cast<BufferComponentOGL*>(comp);
						ogl2::Buffer& buffer = buf.ogl_get_buffer();
						switch(buf.get_resource()->get_access())
						{
							case ResourceAccess::StaticFixed:
								{
									// Create staging buffer and do a copy.
									ogl2::BufferTarget buf_tar;
									if(res->get_flags().contains(ResourceFlag::IndexBuffer))
									{
										buf_tar = ogl2::BufferTarget::Index;
									}
									else
									{
										buf_tar = ogl2::BufferTarget::ShaderStorage;
									}
									ogl2::Buffer staging
									{{
										.target = buf_tar,
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

						if(res->get_access() == ResourceAccess::DynamicFixed)
						{
							// Tell the resource to use the buffer's data. Also copy whatever we had before.
							std::span<const std::byte> initial_data = res->data();
							std::span<std::byte> buffer_byte_data = buf.ogl_get_buffer().map_as<std::byte>();
							std::copy(initial_data.begin(), initial_data.end(), buffer_byte_data.begin());
							res->set_mapped_data(buffer_byte_data);
						}
					}
				break;
				case ResourceType::Image:
					{
						ImageComponentOGL& img = *static_cast<ImageComponentOGL*>(comp);
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
		};
		std::size_t encountered_reference_count = 0;
		for(std::size_t i = 0; i < this->count(); i++)
		{
			IResource* res = this->get(static_cast<tz::HandleValue>(i));
			IComponent* comp = nullptr;
			if(res == nullptr)
			{
				comp = const_cast<IComponent*>(components[encountered_reference_count]);
				this->components.push_back(comp);
				encountered_reference_count++;
				res = comp->get_resource();
				this->set(static_cast<tz::HandleValue>(i), res);
			}
			else
			{
				switch(res->get_type())
				{
					case ResourceType::Buffer:
						this->components.push_back(tz::make_owned<BufferComponentOGL>(*res));
					break;
					case ResourceType::Image:
						this->components.push_back(tz::make_owned<ImageComponentOGL>(*res));
					break;
					default:
						tz_error("Unrecognised ResourceType. Please submit a bug report.");
					break;
				}
				comp = this->components.back().get();
			}
			do_metadata(comp);
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
		TZ_PROFZONE("OpenGL Frontend - RendererOGL ResourceStorage (Fill bindless image buffer)", TZ_PROFCOL_RED);
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
				auto bcomp = static_cast<BufferComponentOGL*>(comp);
				if(bcomp->ogl_is_descriptor_stakeholder())
				{
					bcomp->ogl_get_buffer().bind_to_resource_id(i++);
				}
				else
				{
					bcomp->ogl_get_buffer().basic_bind();
				}
			}
		}
	}

	void ResourceStorage::bind_image_buffer()
	{
		if(!this->bindless_image_storage_buffer.is_null())
		{
			auto buf_res_count = this->resource_count_of(ResourceType::Buffer);
			if(this->try_get_index_buffer() != nullptr)
			{
				buf_res_count--;
			}
			this->bindless_image_storage_buffer.bind_to_resource_id(buf_res_count);
		}
	}

	void ResourceStorage::write_dynamic_images()
	{
		for(auto& component_ptr : this->components)
		{
			tz::gl::IResource* res = component_ptr->get_resource();
			if(res->get_type() == ResourceType::Image && res->get_access() != ResourceAccess::StaticFixed)
			{
				// Get the underlying image, and set its data to whatever the span said it was.
				ogl2::Image& img = static_cast<ImageComponentOGL*>(component_ptr.get())->ogl_get_image();
				img.set_data(res->data());
			}
		}
	}

	IComponent* ResourceStorage::try_get_index_buffer() const
	{
		for(auto& component_ptr : this->components)
		{
			if(component_ptr->get_resource()->get_flags().contains(ResourceFlag::IndexBuffer))
			{
				tz_assert(component_ptr->get_resource()->get_type() == ResourceType::Buffer, "Detected non-buffer resource with ResourceFlag::IndexBuffer which is illegal. Please submit a bug report.");
				return const_cast<IComponent*>(component_ptr.get());
			}
		}
		return nullptr;
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

	bool ShaderManager::is_compute() const
	{
		return this->shader.is_compute();
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
	default_depth_renderbuffer(ogl2::Renderbuffer::null()),
	framebuffer(ogl2::Framebuffer::null())
	{
		if(this->output != nullptr)
		{
			switch(this->output->get_target())
			{
			case OutputTarget::OffscreenImage:
			{
				auto* out = static_cast<ImageOutput*>(this->output);
				tz_assert(!out->has_depth_attachment(), "ImageOutput with depth attachment is not yet implemented.");
				tz::BasicList<ogl2::FramebufferTexture> colour_attachments;
				colour_attachments.resize(out->colour_attachment_count());
				for(std::size_t i = 0; i < colour_attachments.length(); i++)
				{
					colour_attachments[i] = {&out->get_colour_attachment(i).ogl_get_image()};
				}

				this->default_depth_renderbuffer =
				{{
					.format = ogl2::ImageFormat::Depth32_UNorm,
					.dimensions = out->get_colour_attachment(0).get_dimensions()
				}};

				this->framebuffer =
				{ogl2::FramebufferInfo{
					.dimensions = out->get_colour_attachment(0).get_dimensions(),
					.maybe_depth_attachment = {&this->default_depth_renderbuffer},
					.colour_attachments = colour_attachments
				}};
			}
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
		TZ_PROFZONE("OpenGL Frontend - RendererOGL OutputManager (Set Render Target)", TZ_PROFCOL_RED);
		this->framebuffer.bind();
		this->framebuffer.clear();
	}

//--------------------------------------------------------------------------------------------------
	
	RendererOGL::RendererOGL(RendererInfoOGL info):
	vao(),
	resources(info.get_resources(), info.get_components()),
	shader(info.shader()),
	output(info.get_output()),
	clear_colour(info.get_clear_colour()),
	compute_kernel(info.get_compute_kernel()),
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
		TZ_PROFZONE("OpenGL Frontend - RendererOGL Render", TZ_PROFCOL_RED);
		TZ_PROFZONE_GPU("RendererOGL Render", TZ_PROFCOL_RED);

		if(this->shader.is_compute())
		{
			this->resources.write_dynamic_images();
			this->shader.use();
			this->resources.bind_buffers();
			if(this->resources.resource_count_of(ResourceType::Image) > 0)
			{
				this->resources.bind_image_buffer();
			}
			tz_assert(this->resources.try_get_index_buffer() == nullptr, "Compute renderer has an index buffer applied to it. This doesn't make any sense. Please submit a bug report.");
			glDispatchCompute(this->compute_kernel[0], this->compute_kernel[1], this->compute_kernel[2]);
			if(this->get_options().contains(RendererOption::BlockingCompute))
			{
				glClientWaitSync(glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0), GL_SYNC_FLUSH_COMMANDS_BIT, std::numeric_limits<GLuint64>::max());
			}
		}
		else
		{
			this->resources.write_dynamic_images();
			glClearColor(this->clear_colour[0], this->clear_colour[1], this->clear_colour[2], this->clear_colour[3]);
			if(this->options.contains(RendererOption::NoDepthTesting))
			{
				glDisable(GL_DEPTH_TEST);
			}
			else
			{
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_LESS);
			}
			if(this->options.contains(RendererOption::AlphaBlending))
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			else
			{
				glDisable(GL_BLEND);
			}
			this->output.set_render_target();

			this->shader.use();
			this->resources.bind_buffers();
			if(this->resources.resource_count_of(ResourceType::Image) > 0)
			{
				this->resources.bind_image_buffer();
			}
			if(this->resources.try_get_index_buffer() != nullptr)
			{
				const ogl2::Buffer& ibuf = static_cast<BufferComponentOGL*>(this->resources.try_get_index_buffer())->ogl_get_buffer();
				this->vao.draw_indexed(tri_count, ibuf);
			}
			else
			{
				this->vao.draw(this->tri_count);
			}
		}
	}

	void RendererOGL::render(unsigned int tri_count)
	{
		this->tri_count = tri_count;
		this->render();
	}

	void RendererOGL::edit(const RendererEditRequest& edit_request)
	{
		TZ_PROFZONE("OpenGL Backend - RendererOGL Edit", TZ_PROFCOL_RED);
		if(edit_request.component_edits.empty())
		{
			return;
		}
		if(edit_request.compute_edit.has_value())
		{
			this->compute_kernel = edit_request.compute_edit.value().kernel;
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
