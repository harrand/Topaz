#if TZ_OGL
#include "tz/core/profile.hpp"
#include "tz/core/debug.hpp"
#include "tz/dbgui/dbgui.hpp"
#include "tz/gl/impl/opengl/detail/tz_opengl.hpp"
#include "tz/gl/impl/opengl/detail/buffer.hpp"
#include "tz/gl/impl/opengl/detail/draw.hpp"
#include "tz/gl/impl/opengl/renderer.hpp"
#include "tz/gl/impl/opengl/component.hpp"
#include "tz/gl/output.hpp"
#include "tz/gl/resource.hpp"

namespace tz::gl
{
	unsigned int renderer_ogl_base::uid_counter = 0;
	namespace detail
	{
		void initialise_buffer(buffer_component_ogl& bufcomp)
		{
			ogl2::buffer& buffer = bufcomp.ogl_get_buffer();
			iresource* res = bufcomp.get_resource();
			tz::assert(res != nullptr, "buffer_component had null resource");
			switch(res->get_access())
			{
				case resource_access::static_access:
				{
					// Create a staging buffer, write the resource data into it, and then do a buffer copy to the component.
					ogl2::buffer_target tar = ogl2::buffer_target::shader_storage;
					if(res->get_flags().contains(resource_flag::index_buffer))
					{
						tar = ogl2::buffer_target::index;
					}

					ogl2::buffer staging_buffer
					{{
						.target = tar,
						.residency = ogl2::buffer_residency::dynamic,
						.size_bytes = buffer.size()
					}};
					auto staging_data = staging_buffer.map_as<std::byte>();
					auto resource_data = res->data();
					std::copy(resource_data.begin(), resource_data.end(), staging_data.begin());
					ogl2::buffer_helper::copy(staging_buffer, buffer);
				}
				break;
				case resource_access::dynamic_access:
				{
					// Map component buffer and write resource data directly into it, then pass the mapped ptr back into the resource to set it as the new data source.
					auto resdata = res->data();
					auto compdata = buffer.map_as<std::byte>();
					std::copy(resdata.begin(), resdata.end(), compdata.begin());
					res->set_mapped_data(compdata);
				}
				break;
				default:
					tz::error("resource_access for this buffer is not yet implemented");	
				break;
			}
		}
	}

	ResourceStorage::ResourceStorage(std::span<const iresource* const> resources, std::span<const icomponent* const> components):
	AssetStorageCommon<iresource>(resources),
	components(),
	image_handles(),
	bindless_image_storage_buffer(ogl2::buffer::null())
	{
		TZ_PROFZONE("OpenGL Frontend - renderer_ogl ResourceStorage Create", 0xFFAA0000);
		auto do_metadata = [this](icomponent* comp)
		{
			iresource* res = comp->get_resource();
			switch(res->get_type())
			{
				case resource_type::buffer:
					detail::initialise_buffer(*static_cast<buffer_component_ogl*>(comp));
				break;
				case resource_type::image:
					{
						image_component_ogl& img = *static_cast<image_component_ogl*>(comp);
						ogl2::image& image = img.ogl_get_image();
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
					tz::error("Unrecognised resource_type. Please submit a bug report.");
				break;
			}
		};

		std::size_t encountered_reference_count = 0;
		for(std::size_t i = 0; i < this->count(); i++)
		{
			iresource* res = this->get(static_cast<tz::hanval>(i));
			icomponent* comp = nullptr;
			if(res == nullptr)
			{
				comp = const_cast<icomponent*>(components[encountered_reference_count]);
				this->components.push_back(comp);
				encountered_reference_count++;
				res = comp->get_resource();
				this->set(static_cast<tz::hanval>(i), res);
			}
			else
			{
				switch(res->get_type())
				{
					case resource_type::buffer:
						this->components.push_back(tz::make_owned<buffer_component_ogl>(*res));
					break;
					case resource_type::image:
						this->components.push_back(tz::make_owned<image_component_ogl>(*res));
					break;
					default:
						tz::error("Unrecognised resource_type. Please submit a bug report.");
					break;
				}
				comp = this->components.back().get();
			}
			do_metadata(comp);
		}
		this->fill_bindless_image_buffer();
	}

	const icomponent* ResourceStorage::get_component(resource_handle handle) const
	{
		if(handle == tz::nullhand)
		{
			return nullptr;
		}
		return this->components[static_cast<std::size_t>(static_cast<tz::hanval>(handle))].get();
	}

	icomponent* ResourceStorage::get_component(resource_handle handle)
	{
		if(handle == tz::nullhand)
		{
			return nullptr;
		}
		return this->components[static_cast<std::size_t>(static_cast<tz::hanval>(handle))].get();
	}

	unsigned int ResourceStorage::resource_count_of(resource_type type) const
	{
		return std::count_if(this->components.begin(), this->components.end(),
		[type](const auto& component_ptr)
		{
			return component_ptr->get_resource()->get_type() == type;
		});
	}

	void ResourceStorage::fill_bindless_image_buffer()
	{
		TZ_PROFZONE("OpenGL Frontend - renderer_ogl ResourceStorage (Fill bindless image buffer)", 0xFFAA0000);
		if(this->image_handles.empty() || !ogl2::supports_bindless_textures())
		{
			return;
		}
		const std::size_t buf_size = this->image_handles.size() * sizeof(ogl2::image::bindless_handle);
		this->bindless_image_storage_buffer =
		{{
			.target = ogl2::buffer_target::shader_storage,
			.residency = ogl2::buffer_residency::static_fixed,
			.size_bytes = buf_size
		}};
		ogl2::buffer temp_copy_buffer =
		{{
			.target = ogl2::buffer_target::shader_storage,
			.residency = ogl2::buffer_residency::dynamic,
			.size_bytes = buf_size
		}};
		{
			std::span<ogl2::image::bindless_handle> handle_data = temp_copy_buffer.map_as<ogl2::image::bindless_handle>();
			std::copy(this->image_handles.begin(), this->image_handles.end(), handle_data.begin());
		}
		ogl2::buffer_helper::copy(temp_copy_buffer, this->bindless_image_storage_buffer);
	}

	void ResourceStorage::bind_buffers(const render_state& state)
	{
		if(this->resource_count_of(resource_type::buffer) == 0)
		{
			return;
		}
		std::size_t i = 0;
		for(std::size_t j = 0; j < this->components.size(); j++)
		{
			icomponent* comp = this->components[j].get();
			if(comp->get_resource()->get_type() == resource_type::buffer)
			{
				auto bcomp = static_cast<buffer_component_ogl*>(comp);
				// If we're an index or draw-indirect buffer. Don't bind. The draw command will do that.
				if(this->get_component(state.graphics.index_buffer) == comp || this->get_component(state.graphics.draw_buffer) == comp)
				{

				}
				else if(this->get(state.graphics.index_buffer) != bcomp->get_resource() && this->get(state.graphics.draw_buffer) != bcomp->get_resource())
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
		auto buf_res_count = this->resource_count_of(resource_type::buffer);
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
				auto img_nat = static_cast<ogl2::image::NativeType>(this->image_handles[i]);
				#if TZ_DEBUG
					int max_textures;
					glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max_textures);
					tz::assert(std::cmp_less(i, max_textures), "Too many textures bound at once. Max of %d, but we have %zu", max_textures, this->image_handles.size());
				#endif
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
			tz::gl::iresource* res = component_ptr->get_resource();
			if(res->get_type() == resource_type::image && res->get_access() != resource_access::static_access)
			{
				// Get the underlying image, and set its data to whatever the span said it was.
				ogl2::image& img = static_cast<image_component_ogl*>(component_ptr.get())->ogl_get_image();
				img.set_data(res->data());
			}
		}
	}

	void ResourceStorage::set_image_handle(tz::gl::resource_handle h, ogl2::image::bindless_handle bindless_handle)
	{
		this->image_handles[static_cast<std::size_t>(static_cast<tz::hanval>(h))] = bindless_handle;
	}

	void ResourceStorage::reseat_resource_reference(tz::gl::resource_handle h, icomponent* comp)
	{
		iresource* oldres = this->get(h);
		iresource* newres = comp->get_resource();
		tz::assert(oldres->get_type() == newres->get_type());
		tz::assert(newres != nullptr);
		AssetStorageCommon<iresource>::set(h, newres);
		auto resid = static_cast<std::size_t>(static_cast<tz::hanval>(h));
		this->components[resid] = comp;
		if(newres->get_type() == tz::gl::resource_type::image)
		{
			// need to update image the bindless texture handle.
			auto* imgcomp = static_cast<image_component_ogl*>(comp);
			ogl2::image::bindless_handle newh = imgcomp->ogl_get_image().native();
			if(ogl2::supports_bindless_textures())
			{
				newh = imgcomp->ogl_get_image().get_bindless_handle();
			}
			this->set_image_handle(h, newh);
		}
	}

//--------------------------------------------------------------------------------------------------

	ShaderManager::ShaderManager(const shader_info& sinfo):
	shader(this->make_shader(sinfo))
	{

	}

	ShaderManager::ShaderManager():
	shader(ogl2::shader::null())
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

	ogl2::shader& ShaderManager::get_program()
	{
		return this->shader;
	}

	ogl2::shader ShaderManager::make_shader(const shader_info& sinfo) const
	{
		tz::basic_list<ogl2::shader_module_info> modules;
		if(sinfo.has_shader(shader_stage::compute))
		{
			// Compute, we only care about the compute shader.
			modules =
			{
				{
					.type = ogl2::shader_type::compute,
					.code = ogl2::ogl_string(sinfo.get_shader(shader_stage::compute))
				}
			};
		}
		else
		{
			// Graphics, must contain a Vertex and Fragment shader.
			tz::assert(sinfo.has_shader(shader_stage::vertex), "shader_info must contain a non-empty vertex shader if no compute shader is present.");
			tz::assert(sinfo.has_shader(shader_stage::fragment), "shader_info must contain a non-empty fragment shader if no compute shader is present.");
			modules =
			{
				{
					.type = ogl2::shader_type::vertex,
					.code = ogl2::ogl_string(sinfo.get_shader(shader_stage::vertex))
				},
				{
					.type = ogl2::shader_type::fragment,
					.code = ogl2::ogl_string(sinfo.get_shader(shader_stage::fragment))
				}
			};

			if(sinfo.has_shader(shader_stage::tessellation_control) || sinfo.has_shader(shader_stage::tessellation_evaluation))
			{
				tz::assert(sinfo.has_shader(shader_stage::tessellation_control) && sinfo.has_shader(shader_stage::tessellation_evaluation), "Detected a tessellaton shader type, but it was missing its sister. If a control or evaluation shader exists, they both must exist.");
				modules.add
				({
					.type = ogl2::shader_type::tessellation_control,
					.code = ogl2::ogl_string(sinfo.get_shader(shader_stage::tessellation_control))
				});

				modules.add
				({
					.type = ogl2::shader_type::tessellation_evaluation,
					.code = ogl2::ogl_string(sinfo.get_shader(shader_stage::tessellation_evaluation))
				});
			}
		}
		return
		{{
			.modules = modules
		}};
	}

//--------------------------------------------------------------------------------------------------

	OutputManager::OutputManager(const ioutput* output, tz::gl::renderer_options options):
	output(output != nullptr ? output->unique_clone() : nullptr),
	default_depth_renderbuffer(ogl2::render_buffer::null()),
	framebuffer(ogl2::framebuffer::null()),
	options(options)
	{
		if(this->output != nullptr)
		{
			switch(this->output->get_target())
			{
			case output_target::offscreen_image:
			{
				auto* out = static_cast<image_output*>(this->output.get());
				tz::assert(!out->has_depth_attachment(), "image_output with depth attachment is not yet implemented.");
				tz::basic_list<ogl2::framebuffer_texture> colour_attachments;
				colour_attachments.resize(out->colour_attachment_count());
				for(std::size_t i = 0; i < colour_attachments.length(); i++)
				{
					colour_attachments[i] = {&out->get_colour_attachment(i).ogl_get_image()};
				}

				this->default_depth_renderbuffer =
				{{
					.format = ogl2::image_format::Depth32_UNorm,
					.dimensions = out->get_colour_attachment(0).get_dimensions()
				}};

				this->framebuffer =
				{ogl2::framebuffer_info{
					.dimensions = out->get_colour_attachment(0).get_dimensions(),
					.maybe_depth_attachment = {&this->default_depth_renderbuffer},
					.colour_attachments = colour_attachments
				}};
			}
			break;
			case output_target::window:
				// Do nothing. We use null framebuffer.
			break;
			default:
				tz::error("Use of this specific output_target is not yet implemented.");
			break;
			}
		}
	}

	void OutputManager::set_render_target() const
	{
		TZ_PROFZONE("OpenGL Frontend - renderer_ogl OutputManager (Set Render Target)", 0xFFAA0000);
		this->framebuffer.bind();
		if(!this->options.contains(tz::gl::renderer_option::no_clear_output))
		{
			this->framebuffer.clear();
		}
		if(this->output != nullptr && this->output->viewport != tz::gl::viewport_region::null())
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
		if(this->output != nullptr && this->output->scissor != tz::gl::scissor_region::null())
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

	ioutput* OutputManager::get_output()
	{
		return this->output.get();
	}

	const ioutput* OutputManager::get_output() const
	{
		return this->output.get();
	}

//--------------------------------------------------------------------------------------------------
	
	renderer_ogl::renderer_ogl(const renderer_info& info):
	vao(),
	resources(info.get_resources(), info.get_components()),
	shader(info.shader()),
	output(info.get_output(), info.get_options()),
	options(info.get_options()),
	state(info.state()),
	debug_name(info.debug_get_name())
	{
		if(this->options.contains(tz::gl::renderer_option::draw_indirect_count))
		{
			tz::assert(this->state.graphics.draw_buffer != tz::nullhand);
		}
		// Handle debug names for resources.
		#if TZ_DEBUG
			for(std::size_t i = 0; i < this->resource_count(); i++)
			{
				icomponent* comp = this->resources.get_component(static_cast<tz::hanval>(i));
				if(comp->get_resource()->get_type() == resource_type::buffer)
				{
					ogl2::buffer& buf = static_cast<buffer_component_ogl*>(comp)->ogl_get_buffer();
					std::string n = buf.debug_get_name();
					buf.debug_set_name(n + (n.empty() ? "" : " -> ") + this->debug_name + ":B" + std::to_string(i));
				}
				if(comp->get_resource()->get_type() == resource_type::image)
				{
					ogl2::image& img = static_cast<image_component_ogl*>(comp)->ogl_get_image();
					std::string n = img.debug_get_name();
					img.debug_set_name(n + (n.empty() ? "" : " -> ") + this->debug_name + ":I" + std::to_string(i));
				}
			}
			this->shader.get_program().debug_set_name(this->debug_name + ":Shader");
		#endif // TZ_DEBUG
	}

	unsigned int renderer_ogl::resource_count() const
	{
		return this->resources.count();
	}

	const iresource* renderer_ogl::get_resource(resource_handle handle) const
	{
		return this->resources.get(handle);
	}

	iresource* renderer_ogl::get_resource(resource_handle handle)
	{
		return this->resources.get(handle);
	}

	const icomponent* renderer_ogl::get_component(resource_handle handle) const
	{
		return this->resources.get_component(handle);
	}

	icomponent* renderer_ogl::get_component(resource_handle handle)
	{
		return this->resources.get_component(handle);
	}

	ioutput* renderer_ogl::get_output()
	{
		return this->output.get_output();
	}

	const ioutput* renderer_ogl::get_output() const
	{
		return this->output.get_output();
	}

	const renderer_options& renderer_ogl::get_options() const
	{
		return this->options;
	}

	const render_state& renderer_ogl::get_state() const
	{
		return this->state;
	}

	void renderer_ogl::render()
	{
		TZ_PROFZONE("OpenGL Frontend - renderer_ogl Render", 0xFFAA0000);
		tz::assert(!this->is_null(), "Attempting to render with a null renderer. Please submit a bug report.");
		#if TZ_DEBUG
		{
			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, this->debug_name.c_str());
		}
		#endif

		tz::gl::get_device().ogl_gpu_do_waits(renderer_ogl_base::ogl_get_uid());

		if(this->shader.is_compute())
		{
			this->resources.write_dynamic_images();
			this->shader.use();
			this->resources.bind_buffers(this->state);
			if(this->resources.resource_count_of(resource_type::image) > 0)
			{
				this->resources.bind_image_buffer(this->state.graphics.index_buffer != tz::nullhand, this->state.graphics.draw_buffer != tz::nullhand);
			}
			//tz::assert(this->resources.try_get_index_buffer() == nullptr, "Compute renderer has an index buffer applied to it. This doesn't make any sense. Please submit a bug report.");
			{
				auto ker = this->state.compute.kernel;
				glDispatchCompute(ker[0], ker[1], ker[2]);
			}
		}
		else
		{
			ogl2::primitive_topology topology;
			switch(this->state.graphics.topology)
			{
				case tz::gl::graphics_topology::triangles:
					topology = ogl2::primitive_topology::triangles;
				break;
				case tz::gl::graphics_topology::points:
					topology = ogl2::primitive_topology::points;
				break;
				case tz::gl::graphics_topology::triangle_strips:
					topology = ogl2::primitive_topology::triangle_strips;
				break;
				default:
					tz::error("Unrecognised `tz::gl::graphics_topology`.");
					topology = ogl2::primitive_topology::triangles;
				break;
			}
			const bool window_output = this->output.get_output() == nullptr || this->output.get_output()->get_target() == tz::gl::output_target::window;
			this->resources.write_dynamic_images();
			{
				auto col = this->state.graphics.clear_colour;
				glClearColor(col[0], col[1], col[2], col[3]);
			}
			if(this->options.contains(renderer_option::no_depth_testing))
			{
				glDisable(GL_DEPTH_TEST);
			}
			else
			{
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_LESS);
			}
			if(this->state.graphics.culling == tz::gl::graphics_culling::none)
			{
				glDisable(GL_CULL_FACE);
			}
			else
			{
				glEnable(GL_CULL_FACE);
				GLenum cullmode;
				switch(this->state.graphics.culling)
				{
					case tz::gl::graphics_culling::front:
						cullmode = GL_FRONT;
					break;
					default:
					[[fallthrough]];
					case tz::gl::graphics_culling::back:
						cullmode = GL_BACK;
					break;
					case tz::gl::graphics_culling::both:
						cullmode = GL_FRONT_AND_BACK;
					break;
				}
				glCullFace(cullmode);
			}
			if(this->options.contains(renderer_option::alpha_blending))
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
			if(this->resources.resource_count_of(resource_type::image) > 0)
			{
				this->resources.bind_image_buffer(this->state.graphics.index_buffer != tz::nullhand, this->state.graphics.draw_buffer != tz::nullhand);
			}

			glPolygonMode(GL_FRONT_AND_BACK, this->wireframe_mode ? GL_LINE : GL_FILL);

			auto* icomp = this->get_component(this->state.graphics.index_buffer);
			if(icomp != nullptr)
			{
				const ogl2::buffer& ibuf = static_cast<buffer_component_ogl*>(icomp)->ogl_get_buffer();
				auto* dcomp = this->get_component(this->state.graphics.draw_buffer);
				if(dcomp != nullptr)
				{
					const ogl2::buffer& dbuf = static_cast<buffer_component_ogl*>(dcomp)->ogl_get_buffer();
					if(this->options.contains(tz::gl::renderer_option::draw_indirect_count))
					{
						this->vao.draw_indexed_indirect_count(dbuf.size() / sizeof(ogl2::draw_indexed_indirect_command), ibuf, dbuf, static_cast<std::uintptr_t>(sizeof(std::uint32_t)), topology, this->shader.has_tessellation());
					}
					else
					{
						this->vao.draw_indexed_indirect(dbuf.size() / sizeof(ogl2::draw_indexed_indirect_command), ibuf, dbuf, topology, this->shader.has_tessellation());
					}
				}
				else
				{
					this->vao.draw_indexed(this->state.graphics.tri_count, ibuf, topology, this->shader.has_tessellation());
				}
			}
			else
			{
				auto* dcomp = this->get_component(this->state.graphics.draw_buffer);
				if(dcomp != nullptr)
				{
					const ogl2::buffer& dbuf = static_cast<buffer_component_ogl*>(dcomp)->ogl_get_buffer();
					if(this->options.contains(tz::gl::renderer_option::draw_indirect_count))
					{
						this->vao.draw_indirect_count(dbuf.size() / sizeof(ogl2::draw_indirect_command), dbuf, static_cast<std::uintptr_t>(sizeof(std::uint32_t)), topology, this->shader.has_tessellation());
					}
					else
					{
						this->vao.draw_indirect(dbuf.size() / sizeof(ogl2::draw_indirect_command), dbuf, topology, this->shader.has_tessellation());
					}
				}
				else
				{
					this->vao.draw(this->state.graphics.tri_count, topology, this->shader.has_tessellation());
				}
			}
			if(!this->options.contains(tz::gl::renderer_option::no_present) && window_output)
			{
				tz::window().update();
			}
		}
		#if TZ_DEBUG
		{
			glPopDebugGroup();
		}
		#endif
		// If we're doing instant render, block now.
		if(this->get_options().contains(renderer_option::render_wait))
		{
			auto fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, std::numeric_limits<GLuint64>::max());
			glDeleteSync(fence);
		}
		tz::gl::get_device().ogl_register_sync(renderer_ogl_base::ogl_get_uid());
	}

	void renderer_ogl::edit(const renderer_edit_request& edit_request)
	{
		TZ_PROFZONE("OpenGL Backend - renderer_ogl Edit", 0xFFAA0000);
		tz::assert(!this->is_null(), "Attempting to perform an edit on the null renderer. Please submit a bug report.");
		for(const renderer_edit::variant& req : edit_request)
		{
			std::visit([this](auto&& arg)
			{
				using T = std::decay_t<decltype(arg)>;
				if constexpr(std::is_same_v<T, renderer_edit::buffer_resize>)
				{
					auto bufcomp = static_cast<buffer_component_ogl*>(this->get_component(arg.buffer_handle));
					tz::assert(bufcomp != nullptr, "Invalid buffer handle in renderer_edit::buffer_resize");
					if(bufcomp->size() != arg.size)
					{
						bufcomp->resize(arg.size);
					}
				}
				else if constexpr(std::is_same_v<T, renderer_edit::image_resize>)
				{
					auto imgcomp = static_cast<image_component_ogl*>(this->get_component(arg.image_handle));
					tz::assert(imgcomp != nullptr, "Invalid image handle in renderer_edit::image_resize");
					if(imgcomp->get_dimensions() != arg.dimensions)
					{
						imgcomp->resize(arg.dimensions);
						
						ogl2::image::bindless_handle h = imgcomp->ogl_get_image().native();
						if(ogl2::supports_bindless_textures())
						{
							h = imgcomp->ogl_get_image().get_bindless_handle();
						}
						this->resources.set_image_handle(arg.image_handle, h);
						this->resources.fill_bindless_image_buffer();
					}
				}
				else if constexpr(std::is_same_v<T, renderer_edit::resource_write>)
				{
					icomponent* comp = this->get_component(arg.resource);
					iresource* res = comp->get_resource();
					switch(res->get_access())
					{
						default:
							tz::error("Unrecognised resource access");
							[[fallthrough]];
						case resource_access::static_access:
							switch(res->get_type())
							{
								case resource_type::buffer:
								{
									ogl2::buffer& buffer = static_cast<buffer_component_ogl*>(comp)->ogl_get_buffer();
									ogl2::buffer staging_buffer
									{{
										.target = ogl2::buffer_target::uniform,
										.residency = ogl2::buffer_residency::dynamic,
										.size_bytes = arg.data.size_bytes()
									}};
									{
										void* ptr = staging_buffer.map();
										std::memcpy(ptr, arg.data.data(), arg.data.size_bytes());
										staging_buffer.unmap();
									}
									ogl2::buffer_helper::copy(staging_buffer, buffer);
								}
								break;
								case resource_type::image:
									ogl2::image& image = static_cast<image_component_ogl*>(comp)->ogl_get_image();
									ogl2::image staging_image
									{{
										.format = image.get_format(),
										.dimensions = image.get_dimensions(),
										.shader_sampler = image.get_sampler()
									}};
									staging_image.set_data(arg.data);
									ogl2::image_helper::copy(staging_image, image);

								break;
							}
						break;
						case resource_access::dynamic_access:
							tz::report("Received component write edit request for resource handle %zu, which is being carried out, but is unnecessary because the resource has dynamic access, meaning you can just mutate data().", static_cast<std::size_t>(static_cast<tz::hanval>(arg.resource)));
							std::span<std::byte> data = res->data_as<std::byte>();
							std::copy(arg.data.begin(), arg.data.end(), data.begin() + arg.offset);
						break;
					}
				}
				else if constexpr(std::is_same_v<T, renderer_edit::compute_config>)
				{
					this->state.compute.kernel = arg.kernel;
				}
				else if constexpr(std::is_same_v<T, renderer_edit::render_config>)
				{
					if(arg.wireframe_mode.has_value())
					{
						this->wireframe_mode = arg.wireframe_mode.value();
					}
					if(arg.clear_colour.has_value())
					{
						this->state.graphics.clear_colour = arg.clear_colour.value();
					}
					if(arg.tri_count.has_value())
					{
						this->state.graphics.tri_count = arg.tri_count.value();
					}
					if(arg.culling.has_value())
					{
						this->state.graphics.culling = arg.culling.value();
					}
				}
				else if constexpr(std::is_same_v<T, renderer_edit::resource_reference>)
				{
					this->resources.reseat_resource_reference(arg.resource, arg.component);
				}
				else if constexpr(std::is_same_v<T, renderer_edit::mark_dirty>)
				{
					// mark dirty has no impl for ogl! hooray!
				}
				else
				{
					tz::error("renderer_edit requested that is not yet supported.");
				}
			}, req);
		}
	}

	void renderer_ogl::dbgui()
	{
		common_renderer_dbgui(*this);
	}

	std::string_view renderer_ogl::debug_get_name() const
	{
		return this->debug_name;
	}

	renderer_ogl renderer_ogl::null()
	{
		return {};
	}

	bool renderer_ogl::is_null() const
	{
		return this->is_null_value;
	}

	renderer_ogl::renderer_ogl():
	vao(),
	resources({}, {}),
	shader(),
	output(nullptr, {}),
	options(),
	state(),
	debug_name("Null renderer")
	{
		this->is_null_value = true;
	}
}

#endif // TZ_OGL
