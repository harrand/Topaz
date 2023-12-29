#ifndef TOPAZ_GL2_IMPL_FRONTEND_OGL2_RENDERER_HPP
#define TOPAZ_GL2_IMPL_FRONTEND_OGL2_RENDERER_HPP
#if TZ_OGL
#include "tz/core/data/maybe_owned_list.hpp"
#include "tz/gl/api/renderer.hpp"
#include "tz/gl/api/component.hpp"
#include "tz/gl/impl/opengl/detail/vertex_array.hpp"
#include "tz/gl/impl/opengl/detail/shader.hpp"
#include "tz/gl/impl/opengl/detail/framebuffer.hpp"
#include "tz/gl/impl/opengl/detail/buffer.hpp"

namespace tz::gl
{
	using namespace tz::gl;

	/**
	 * @ingroup tz_gl2_graphicsapi_ogl_frontend
	 * @defgroup tz_gl2_graphicsapi_ogl_frontend_renderer renderer Implementation
	 * Documentation for the OpenGL Frontend implementation of @ref renderer_type.
	 */

	/**
	 * @ingroup tz_gl2_graphicsapi_ogl_frontend_renderer
	 * Copies all resource data from upon creation and handles resource and component lifetimes. Also exposes said copied resources and components to the renderer.
	 */
	class ResourceStorage : public tz::maybe_owned_list<iresource>
	{
	public:
		/**
		 * Create storage for a set of existing resources.
		 *
		 * All existing ResourceHandles referencing any of the provided resources will continue to be valid when passed to the renderer_ogl. However, it will reference the copy of said resource which is created during construction of this object. This means users are able to and encouraged to cache their ResourceHandles when populating renderer_info.
		 *
		 * @param resources A view into an array of existing resources. All of these will be copies into a separate storage, meaning the elements of the span are allowed to reach the end of their lifetime after the storage has been constructed, because they will have been cloned.
		 */
		ResourceStorage(std::span<const iresource* const> resources, std::span<const icomponent* const> components);
		/**
		 * Retrieve the component (read-only) which stores the corresponding opengl backend objects for the resource corresponding to the handle.
		 * @param handle Handle whose resource's component needs to be retrieved. The handle must have referred to one of the initial resources passed to the constructor, otherwise the behaviour is undefined.
		 */
		const icomponent* get_component(resource_handle handle) const;
		/**
		 * Retrieve the component which stores the corresponding opengl backend objects for the resource corresponding to the handle.
		 * @param handle Handle whose resource's component needs to be retrieved. The handle must have referred to one of the initial resources passed to the constructor, otherwise the behaviour is undefined.
		 */
		icomponent* get_component(resource_handle handle);
		/**
		 * Retrieve the number of resources stored of the given type.
		 * @param type Type whose quantity should be retrieved.
		 * @return number of resources matching the provided type.
		 */
		unsigned int resource_count_of(resource_type type) const;
		/**
		 * Bind all buffer resources to their expected resource ids.
		 */
		void bind_buffers(const render_state& state);
		/**
		 * images are converted into bindless texture handles which are then all stored within a secret bespoke SSBO (this does not count as a buffer resource however). This binds that SSBO to the resource id equal to the list of images (this will be equal to the total number of buffer resources).
		 */
		void bind_image_buffer(bool has_index_buffer, bool has_draw_buffer);
		void write_dynamic_images();
		void set_image_handle(tz::gl::resource_handle h, ogl2::image::bindless_handle bindless_handle);
		void reseat_resource_reference(tz::gl::resource_handle h, icomponent* comp);
		void fill_bindless_image_buffer();
	private:

		/// Stores components corresponding to each resource.
		std::vector<tz::maybe_owned_ptr<icomponent>> components;
		/// Stores a bindless texture handle for each image resource.
		std::vector<ogl2::image::bindless_handle> image_handles;
		// Shader has an array of texture samplers in tzsl. tzslc compiles this down to actually a storage buffer containing a variable array of texture samplers. This is that buffer.
		ogl2::buffer bindless_image_storage_buffer;
	};

	/**
	 * @ingroup tz_gl2_graphicsapi_ogl_frontend_renderer
	 * Stores the shader program and allows the renderer to use it before emitting a draw call.
	 */
	class ShaderManager
	{
	public:
		/**
		 * Construct the shader program from the information provided.
		 */
		ShaderManager(const shader_info& sinfo);
		/**
		 * Construct a null shader manager. It is an error to do any render/compute work with a null shader manager.
		 */
		ShaderManager();
		/**
		 * Use the shader program, meaning it will be used in the next draw call.
		 */
		void use();
		bool is_compute() const;
		bool has_tessellation() const;

		ogl2::shader& get_program();

	private:
		ogl2::shader make_shader(const shader_info& sinfo) const;

		ogl2::shader shader;
	};

	/**
	 * @ingroup tz_gl2_graphicsapi_ogl_frontend_renderer
	 * Stores information about the output target, and a framebuffer which either points to an offscreen image for render-to-texture (not yet implemented), or the main window framebuffer via ogl2::framebuffer::null().
	 */
	class OutputManager
	{
	public:
		/**
		 * Create the output target based upon the renderer information.
		 */
		OutputManager(const ioutput* output, tz::gl::renderer_options options);
		/**
		 * Set this as the render target, causing subsequent draw calls to render into whatever the output is.
		 */
		void set_render_target() const;
		ioutput* get_output();
		const ioutput* get_output() const;
	private:
		std::unique_ptr<ioutput> output;
		ogl2::render_buffer default_depth_renderbuffer;
		ogl2::framebuffer framebuffer;
		tz::gl::renderer_options options;
	};

	struct renderer_ogl_base
	{
		protected:
		// devices have this concept of renderer handles, but they are not guaranteed to be unique (e.g if renderer handle 2 is deleted and a new renderer is created, that will also have handle 2.)
		// this is a uid which will uniquely identify ths current renderer. renderers need to have their own identity because other manager classes (mainly device_vulkan2) does bookkeeping for renderers and needs to know who is who.
		static unsigned int uid_counter;
		unsigned int uid = uid_counter++;
	public:
		unsigned int ogl_get_uid() const{return this->uid;}
	};

	/**
	 * @ingroup tz_gl2_graphicsapi_ogl_frontend_renderer
	 * renderer implementation which heavily calls into the backend at @ref tz_gl_ogl2.
	 */
	class renderer_ogl : public renderer_ogl_base
	{
	public:
		/**
		 * Create a new renderer.
		 * @param info User-exposed class which describes how many resources etc. we have and a high-level description of where we expect to render into.
		 */
		renderer_ogl(const renderer_info& info);
		// Satisfies renderer_type.
		/**
		 * Retrieve the number of resources.
		 */
		unsigned int resource_count() const;
		/**
		 * Retrieve the resource (read-only) corresponding to the given handle.
		 * @param Handle handle returned from a call to a renderer_info's `add_resource`. If this handle came from a renderer_info different to the one we were provided, the behaviour is undefined.
		 * @return Pointer to the resource.
		 */
		const iresource* get_resource(resource_handle handle) const;
		/**
		 * Retrieve the resource corresponding to the given handle.
		 * @param Handle handle returned from a call to a renderer_info's `add_resource`. If this handle came from a renderer_info different to the one we were provided, the behaviour is undefined.
		 * @return Pointer to the resource.
		 */
		iresource* get_resource(resource_handle handle);
		/**
		 * Retrieve the component sourcing the resource (read-only) corresponding to the given handle.
		 * @param Handle handle returned from a call to a renderer_info's `add_resource`. If this handle came from a renderer_info different to the one we were provided, the behaviour is undefined.
		 * @return Pointer to the resource's underlying component.
		 */
		const icomponent* get_component(resource_handle handle) const;
		/**
		 * Retrieve the component sourcing the resource corresponding to the given handle.
		 * @param Handle handle returned from a call to a renderer_info's `add_resource`. If this handle came from a renderer_info different to the one we were provided, the behaviour is undefined.
		 * @return Pointer to the resource's underlying component.
		 */
		icomponent* get_component(resource_handle handle);
		ioutput* get_output();
		const ioutput* get_output() const;
		/**
		 * Retrieve options denoting extra features used by the renderer.
		 */
		const renderer_options& get_options() const;
		/**
		 * Retrieve current state of the renderer.
		 */
		const render_state& get_state() const;
		/**
		 * Invoke the renderer, emitting a single draw call of a set number of triangles. The number of triangles rendered is equal to the number of triangles rendered in the previous draw-call. If this is the first draw, zero triangles are rendered.
		 */
		void render();
		/**
		 * Confirm changes to a renderer.
		 *
		 * Editing renderers is expensive, so it should only be done if absolutely necessary. If you are editing renderers on a per-frame basis, consider creating multiple different renderers upfront for each hot-path and switching between them as necessary instead.
		 * @param edit_request Structure specifying which edits to make.
		 */
		void edit(const renderer_edit_request& edit_request);

		void dbgui();
		std::string_view debug_get_name() const;

		// Satisfies nullable.
		static renderer_ogl null();
		bool is_null() const;
	private:
		renderer_ogl();

		ogl2::vertex_array vao;
		ResourceStorage resources;
		ShaderManager shader;
		OutputManager output;
		renderer_options options;
		render_state state;
		std::string debug_name;
		bool wireframe_mode = false;
		bool is_null_value = false;
	};
	static_assert(renderer_type<renderer_ogl>);
}

#endif // TZ_OGL
#endif // TOPAZ_GL2_IMPL_FRONTEND_OGL2_RENDERER_HPP
