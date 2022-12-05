#if TZ_OGL
#include "hdk/profile.hpp"
#include "hdk/debug.hpp"
#include "tz/dbgui/dbgui.hpp"
#include "tz/gl/impl/backend/ogl2/tz_opengl.hpp"
#include "tz/gl/impl/backend/ogl2/buffer.hpp"
#include "tz/gl/impl/backend/ogl2/draw.hpp"
#include "tz/gl/impl/frontend/ogl2/renderer.hpp"
#include "tz/gl/impl/frontend/ogl2/component.hpp"
#include "tz/gl/output.hpp"
#include "tz/gl/resource.hpp"

namespace tz::gl
{
	namespace detail
	{
		void initialise_buffer(BufferComponentOGL& bufcomp)
		{
			ogl2::Buffer& buffer = bufcomp.ogl_get_buffer();
			IResource* res = bufcomp.get_resource();
			hdk::assert(res != nullptr, "BufferComponent had null resource");
			switch(res->get_access())
			{
				case ResourceAccess::StaticFixed:
				{
					// Create a staging buffer, write the resource data into it, and then do a buffer copy to the component.
					ogl2::BufferTarget tar = ogl2::BufferTarget::ShaderStorage;
					if(res->get_flags().contains(ResourceFlag::IndexBuffer))
					{
						tar = ogl2::BufferTarget::Index;
					}

					ogl2::Buffer staging_buffer
					{{
						.target = tar,
						.residency = ogl2::BufferResidency::Dynamic,
						.size_bytes = buffer.size()
					}};
					auto staging_data = staging_buffer.map_as<std::byte>();
					auto resource_data = res->data();
					std::copy(resource_data.begin(), resource_data.end(), staging_data.begin());
					ogl2::buffer::copy(staging_buffer, buffer);
				}
				break;
				case ResourceAccess::DynamicFixed:
				[[fallthrough]];
				case ResourceAccess::DynamicVariable:
				{
					// Map component buffer and write resource data directly into it, then pass the mapped ptr back into the resource to set it as the new data source.
					auto resdata = res->data();
					auto compdata = buffer.map_as<std::byte>();
					std::copy(resdata.begin(), resdata.end(), compdata.begin());
					res->set_mapped_data(compdata);
				}
				break;
				default:
					hdk::error("ResourceAccess for this buffer is not yet implemented");	
				break;
			}
		}
	}

	ResourceStorage::ResourceStorage(std::span<const IResource* const> resources, std::span<const IComponent* const> components):
	AssetStorageCommon<IResource>(resources),
	components(),
	image_handles(),
	bindless_image_storage_buffer(ogl2::Buffer::null())
	{
		HDK_PROFZONE("OpenGL Frontend - RendererOGL ResourceStorage Create", 0xFFAA0000);
		auto do_metadata = [this](IComponent* comp)
		{
			IResource* res = comp->get_resource();
			switch(res->get_type())
			{
				case ResourceType::Buffer:
					detail::initialise_buffer(*static_cast<BufferComponentOGL*>(comp));
				break;
				case ResourceType::Image:
					{
						ImageComponentOGL& img = *static_cast<ImageComponentOGL*>(comp);
						ogl2::Image& image = img.ogl_get_image();
						image.set_data(img.get_resource()->data());
						if(ogl2::supports_bindless_textures())
						{
							// Note: If we are a resource reference, this component has probably already been made bindless in the past, and making a bindless image bindless again will assert.
							if(!image.is_bindless())
							{
								image.make_bindless();
							}
							this->image_handles.push_back(img.ogl_get_image().get_bindless_handle());
						}
						else
						{
							this->image_handles.push_back(img.ogl_get_image().native());
						}
					}
				break;
				default:
					hdk::error("Unrecognised ResourceType. Please submit a bug report.");
				break;
			}
		};

		std::size_t encountered_reference_count = 0;
		for(std::size_t i = 0; i < this->count(); i++)
		{
			IResource* res = this->get(static_cast<hdk::hanval>(i));
			IComponent* comp = nullptr;
			if(res == nullptr)
			{
				comp = const_cast<IComponent*>(components[encountered_reference_count]);
				this->components.push_back(comp);
				encountered_reference_count++;
				res = comp->get_resource();
				this->set(static_cast<hdk::hanval>(i), res);
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
						hdk::error("Unrecognised ResourceType. Please submit a bug report.");
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
		if(handle == hdk::nullhand)
		{
			return nullptr;
		}
		return this->components[static_cast<std::size_t>(static_cast<hdk::hanval>(handle))].get();
	}

	IComponent* ResourceStorage::get_component(ResourceHandle handle)
	{
		if(handle == hdk::nullhand)
		{
			return nullptr;
		}
		return this->components[static_cast<std::size_t>(static_cast<hdk::hanval>(handle))].get();
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
		HDK_PROFZONE("OpenGL Frontend - RendererOGL ResourceStorage (Fill bindless image buffer)", 0xFFAA0000);
		if(this->image_handles.empty() || !ogl2::supports_bindless_textures())
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

	void ResourceStorage::bind_buffers(const RendererState& state)
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
				// If we're an index or draw-indirect buffer. Don't bind. The draw command will do that.
				if(this->get_component(state.graphics.index_buffer) == comp || this->get_component(state.graphics.draw_buffer) == comp)
				{

				}
				else if(bcomp->ogl_is_descriptor_stakeholder())
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

	void ResourceStorage::bind_image_buffer(bool has_index_buffer, bool has_draw_buffer)
	{
		auto buf_res_count = this->resource_count_of(ResourceType::Buffer);
		if(has_index_buffer)
		{
			buf_res_count--;
		}
		if(has_draw_buffer)
		{
			buf_res_count--;
		}

		if(ogl2::supports_bindless_textures())
		{
			if(!this->bindless_image_storage_buffer.is_null())
			{
				this->bindless_image_storage_buffer.bind_to_resource_id(buf_res_count);
			}
		}
		else
		{
			// Just set uniforms.
			std::vector<int> uniforms;
			uniforms.resize(this->image_handles.size());

			for(std::size_t i = 0; i < this->image_handles.size(); i++)
			{
				auto img_nat = static_cast<ogl2::Image::NativeType>(this->image_handles[i]);
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(GL_TEXTURE_2D, img_nat);

				uniforms[i] = i;
			}
			glUniform1iv(buf_res_count, uniforms.size(), uniforms.data());
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

	void ResourceStorage::set_image_handle(tz::gl::ResourceHandle h, ogl2::Image::BindlessTextureHandle bindless_handle)
	{
		this->image_handles[static_cast<std::size_t>(static_cast<hdk::hanval>(h))] = bindless_handle;
	}

//--------------------------------------------------------------------------------------------------

	ShaderManager::ShaderManager(const ShaderInfo& sinfo):
	shader(this->make_shader(sinfo))
	{

	}

	ShaderManager::ShaderManager():
	shader(ogl2::Shader::null())
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

	bool ShaderManager::has_tessellation() const
	{
		return this->shader.has_tessellation();
	}

	ogl2::Shader& ShaderManager::get_program()
	{
		return this->shader;
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
			hdk::assert(sinfo.has_shader(ShaderStage::Vertex), "ShaderInfo must contain a non-empty vertex shader if no compute shader is present.");
			hdk::assert(sinfo.has_shader(ShaderStage::Fragment), "ShaderInfo must contain a non-empty fragment shader if no compute shader is present.");
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

			if(sinfo.has_shader(ShaderStage::TessellationControl) || sinfo.has_shader(ShaderStage::TessellationEvaluation))
			{
				hdk::assert(sinfo.has_shader(ShaderStage::TessellationControl) && sinfo.has_shader(ShaderStage::TessellationEvaluation), "Detected a tessellaton shader type, but it was missing its sister. If a control or evaluation shader exists, they both must exist.");
				modules.add
				({
					.type = ogl2::ShaderType::TessellationControl,
					.code = ogl2::OGLString(sinfo.get_shader(ShaderStage::TessellationControl))
				});

				modules.add
				({
					.type = ogl2::ShaderType::TessellationEvaluation,
					.code = ogl2::OGLString(sinfo.get_shader(ShaderStage::TessellationEvaluation))
				});
			}
		}
		return
		{{
			.modules = modules
		}};
	}

//--------------------------------------------------------------------------------------------------

	OutputManager::OutputManager(const IOutput* output, tz::gl::RendererOptions options):
	output(output != nullptr ? output->unique_clone() : nullptr),
	default_depth_renderbuffer(ogl2::Renderbuffer::null()),
	framebuffer(ogl2::Framebuffer::null()),
	options(options)
	{
		if(this->output != nullptr)
		{
			switch(this->output->get_target())
			{
			case OutputTarget::OffscreenImage:
			{
				auto* out = static_cast<ImageOutput*>(this->output.get());
				hdk::assert(!out->has_depth_attachment(), "ImageOutput with depth attachment is not yet implemented.");
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
				hdk::error("Use of this specific OutputTarget is not yet implemented.");
			break;
			}
		}
	}

	void OutputManager::set_render_target() const
	{
		HDK_PROFZONE("OpenGL Frontend - RendererOGL OutputManager (Set Render Target)", 0xFFAA0000);
		this->framebuffer.bind();
		if(!this->options.contains(tz::gl::RendererOption::NoClearOutput))
		{
			this->framebuffer.clear();
		}
		if(this->output != nullptr && this->output->viewport != tz::gl::ViewportRegion::null())
		{
			auto x = static_cast<GLint>(this->output->viewport.offset[0]);
			auto y = static_cast<GLint>(this->output->viewport.offset[1]);
			auto width = static_cast<GLsizei>(this->output->viewport.extent[0]);
			auto height = static_cast<GLsizei>(this->output->viewport.extent[1]);
			glViewport(x, y, width, height);
		}
		else
		{
			glViewport(0, 0, this->framebuffer.get_dimensions()[0], this->framebuffer.get_dimensions()[1]);
		}
		if(this->output != nullptr && this->output->scissor != tz::gl::ScissorRegion::null())
		{
			glEnable(GL_SCISSOR_TEST);
			auto x = static_cast<GLint>(this->output->scissor.offset[0]);
			auto y = static_cast<GLint>(this->output->scissor.offset[1]);
			auto width = static_cast<GLsizei>(this->output->scissor.extent[0]);
			auto height = static_cast<GLsizei>(this->output->scissor.extent[1]);
			glScissor(x, y, width, height);
		}
		else
		{
			glDisable(GL_SCISSOR_TEST);
		}
	}

	IOutput* OutputManager::get_output()
	{
		return this->output.get();
	}

	const IOutput* OutputManager::get_output() const
	{
		return this->output.get();
	}

//--------------------------------------------------------------------------------------------------
	
	RendererOGL::RendererOGL(const RendererInfoOGL& info):
	vao(),
	resources(info.get_resources(), info.get_components()),
	shader(info.shader()),
	output(info.get_output(), info.get_options()),
	clear_colour(info.get_clear_colour()),
	compute_kernel(info.get_compute_kernel()),
	options(info.get_options()),
	state(info.state()),
	debug_name(info.debug_get_name())
	{
		// Handle debug names for resources.
		#if HDK_DEBUG
			for(std::size_t i = 0; i < this->resource_count(); i++)
			{
				IComponent* comp = this->resources.get_component(static_cast<hdk::hanval>(i));
				if(comp->get_resource()->get_type() == ResourceType::Buffer)
				{
					ogl2::Buffer& buf = static_cast<BufferComponentOGL*>(comp)->ogl_get_buffer();
					std::string n = buf.debug_get_name();
					buf.debug_set_name(n + (n.empty() ? "" : " -> ") + this->debug_name + ":B" + std::to_string(i));
				}
				if(comp->get_resource()->get_type() == ResourceType::Image)
				{
					ogl2::Image& img = static_cast<ImageComponentOGL*>(comp)->ogl_get_image();
					std::string n = img.debug_get_name();
					img.debug_set_name(n + (n.empty() ? "" : " -> ") + this->debug_name + ":I" + std::to_string(i));
				}
			}
			this->shader.get_program().debug_set_name(this->debug_name + ":Shader");
		#endif // HDK_DEBUG
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

	IOutput* RendererOGL::get_output()
	{
		return this->output.get_output();
	}

	const IOutput* RendererOGL::get_output() const
	{
		return this->output.get_output();
	}

	const RendererOptions& RendererOGL::get_options() const
	{
		return this->options;
	}

	const RendererState& RendererOGL::get_state() const
	{
		return this->state;
	}

	void RendererOGL::render()
	{
		HDK_PROFZONE("OpenGL Frontend - RendererOGL Render", 0xFFAA0000);
		hdk::assert(!this->is_null(), "Attempting to render with a null renderer. Please submit a bug report.");
		#if HDK_DEBUG
		{
			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, this->debug_name.c_str());
		}
		#endif

		if(this->shader.is_compute())
		{
			this->resources.write_dynamic_images();
			this->shader.use();
			this->resources.bind_buffers(this->state);
			if(this->resources.resource_count_of(ResourceType::Image) > 0)
			{
				this->resources.bind_image_buffer(this->state.graphics.index_buffer != hdk::nullhand, this->state.graphics.draw_buffer != hdk::nullhand);
			}
			//hdk::assert(this->resources.try_get_index_buffer() == nullptr, "Compute renderer has an index buffer applied to it. This doesn't make any sense. Please submit a bug report.");
			glDispatchCompute(this->compute_kernel[0], this->compute_kernel[1], this->compute_kernel[2]);
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
			this->resources.bind_buffers(this->state);
			if(this->resources.resource_count_of(ResourceType::Image) > 0)
			{
				this->resources.bind_image_buffer(this->state.graphics.index_buffer != hdk::nullhand, this->state.graphics.draw_buffer != hdk::nullhand);
			}

			glPolygonMode(GL_FRONT_AND_BACK, this->wireframe_mode ? GL_LINE : GL_FILL);

			auto* icomp = this->get_component(this->state.graphics.index_buffer);
			if(icomp != nullptr)
			{
				const ogl2::Buffer& ibuf = static_cast<BufferComponentOGL*>(icomp)->ogl_get_buffer();
				auto* dcomp = this->get_component(this->state.graphics.draw_buffer);
				if(dcomp != nullptr)
				{
					const ogl2::Buffer& dbuf = static_cast<BufferComponentOGL*>(dcomp)->ogl_get_buffer();
					this->vao.draw_indexed_indirect(dbuf.size() / sizeof(ogl2::DrawIndexedIndirectCommand), ibuf, dbuf, this->shader.has_tessellation());
				}
				else
				{
					this->vao.draw_indexed(tri_count, ibuf, this->shader.has_tessellation());
				}
			}
			else
			{
				auto* dcomp = this->get_component(this->state.graphics.draw_buffer);
				if(dcomp != nullptr)
				{
					const ogl2::Buffer& dbuf = static_cast<BufferComponentOGL*>(dcomp)->ogl_get_buffer();
					this->vao.draw_indirect(dbuf.size() / sizeof(ogl2::DrawIndirectCommand), dbuf, this->shader.has_tessellation());
				}
				else
				{
					this->vao.draw(this->tri_count, this->shader.has_tessellation());
				}
			}
			if(!this->options.contains(tz::gl::RendererOption::NoPresent))
			{
				glfwSwapBuffers(tz::window().get_middleware_handle());
			}
		}
		#if HDK_DEBUG
		{
			glPopDebugGroup();
		}
		#endif
		// If we're doing instant render, block now.
		if(this->get_options().contains(RendererOption::RenderWait))
		{
			auto fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, std::numeric_limits<GLuint64>::max());
			glDeleteSync(fence);
		}
	}

	void RendererOGL::render(unsigned int tri_count)
	{
		this->tri_count = tri_count;
		this->render();
	}

	void RendererOGL::edit(const RendererEditRequest& edit_request)
	{
		HDK_PROFZONE("OpenGL Backend - RendererOGL Edit", 0xFFAA0000);
		hdk::assert(!this->is_null(), "Attempting to perform an edit on the null renderer. Please submit a bug report.");
		for(const RendererEdit::Variant& req : edit_request)
		{
			std::visit([this](auto&& arg)
			{
				using T = std::decay_t<decltype(arg)>;
				if constexpr(std::is_same_v<T, RendererEdit::BufferResize>)
				{
					auto bufcomp = static_cast<BufferComponentOGL*>(this->get_component(arg.buffer_handle));
					hdk::assert(bufcomp != nullptr, "Invalid buffer handle in RendererEdit::BufferResize");
					if(bufcomp->size() != arg.size)
					{
						bufcomp->resize(arg.size);
					}
				}
				if constexpr(std::is_same_v<T, RendererEdit::ImageResize>)
				{
					auto imgcomp = static_cast<ImageComponentOGL*>(this->get_component(arg.image_handle));
					hdk::assert(imgcomp != nullptr, "Invalid image handle in RendererEdit::ImageResize");
					if(imgcomp->get_dimensions() != arg.dimensions)
					{
						imgcomp->resize(arg.dimensions);
						
						ogl2::Image::BindlessTextureHandle h = imgcomp->ogl_get_image().native();
						if(ogl2::supports_bindless_textures())
						{
							h = imgcomp->ogl_get_image().get_bindless_handle();
						}
						this->resources.set_image_handle(arg.image_handle, h);
					}
				}
				if constexpr(std::is_same_v<T, RendererEdit::ResourceWrite>)
				{
					IComponent* comp = this->get_component(arg.resource);
					IResource* res = comp->get_resource();
					switch(res->get_access())
					{
						case ResourceAccess::StaticFixed:
							switch(res->get_type())
							{
								case ResourceType::Buffer:
								{
									ogl2::Buffer& buffer = static_cast<BufferComponentOGL*>(comp)->ogl_get_buffer();
									ogl2::Buffer staging_buffer
									{{
										.target = ogl2::BufferTarget::Uniform,
										.residency = ogl2::BufferResidency::Dynamic,
										.size_bytes = res->data().size_bytes()
									}};
									{
										void* ptr = staging_buffer.map();
										std::memcpy(ptr, arg.data.data(), arg.data.size_bytes());
										staging_buffer.unmap();
									}
									ogl2::buffer::copy(staging_buffer, buffer);
								}
								break;
								case ResourceType::Image:
									ogl2::Image& image = static_cast<ImageComponentOGL*>(comp)->ogl_get_image();
									ogl2::Image staging_image
									{{
										.format = image.get_format(),
										.dimensions = image.get_dimensions(),
										.sampler = image.get_sampler()
									}};
									staging_image.set_data(arg.data);
									ogl2::image::copy(staging_image, image);

								break;
							}
						break;
						default:
							hdk::report("Received component write edit request for resource handle %zu, which is being carried out, but is unnecessary because the resource has dynamic access, meaning you can just mutate data().", static_cast<std::size_t>(static_cast<hdk::hanval>(arg.resource)));
							std::span<std::byte> data = res->data_as<std::byte>();
							std::copy(arg.data.begin(), arg.data.end(), data.begin() + arg.offset);
						break;
					}
				}
				if constexpr(std::is_same_v<T, RendererEdit::ComputeConfig>)
				{
					this->compute_kernel = arg.kernel;
				}
				if constexpr(std::is_same_v<T, RendererEdit::RenderConfig>)
				{
					this->wireframe_mode = arg.wireframe_mode;
				}
			}, req);
		}
	}

	void RendererOGL::dbgui()
	{
		common_renderer_dbgui(*this);
	}

	std::string_view RendererOGL::debug_get_name() const
	{
		return this->debug_name;
	}

	RendererOGL RendererOGL::null()
	{
		return {};
	}

	bool RendererOGL::is_null() const
	{
		return this->is_null_value;
	}

	RendererOGL::RendererOGL():
	vao(),
	resources({}, {}),
	shader(),
	output(nullptr, {}),
	clear_colour(),
	compute_kernel(),
	options(),
	state(),
	debug_name("Null Renderer")
	{
		this->is_null_value = true;
	}
}

#endif // TZ_OGL
