#ifndef TOPAZ_GL2_IMPL_FRONTEND_OGL2_RENDERER_HPP
#define TOPAZ_GL2_IMPL_FRONTEND_OGL2_RENDERER_HPP
#if TZ_OGL
#include "gl/api/renderer.hpp"
#include "gl/api/component.hpp"
#include "gl/impl/frontend/common/renderer.hpp"
#include "gl/impl/backend/ogl2/vertex_array.hpp"
#include "gl/impl/backend/ogl2/shader.hpp"
#include "gl/impl/backend/ogl2/framebuffer.hpp"
#include "gl/impl/backend/ogl2/buffer.hpp"

namespace tz::gl
{
	using namespace tz::gl;

	/**
	 * @ingroup tz_gl2_graphicsapi_ogl_frontend
	 * @defgroup tz_gl2_graphicsapi_ogl_frontend_renderer Renderer Implementation
	 * Documentation for the OpenGL Frontend implementation of @ref RendererType.
	 */

	/**
	 * @ingroup tz_gl2_graphicsapi_ogl_frontend_renderer
	 * Copies all resource data from upon creation and handles resource and component lifetimes. Also exposes said copied resources and components to the renderer.
	 */
	class ResourceStorage : public AssetStorageCommon<IResource>
	{
	public:
		/**
		 * Create storage for a set of existing resources.
		 *
		 * All existing ResourceHandles referencing any of the provided resources will continue to be valid when passed to the RendererOGL. However, it will reference the copy of said resource which is created during construction of this object. This means users are able to and encouraged to cache their ResourceHandles when populating RendererInfoOGL.
		 *
		 * @param resources A view into an array of existing resources. All of these will be copies into a separate storage, meaning the elements of the span are allowed to reach the end of their lifetime after the storage has been constructed, because they will have been cloned.
		 */
		ResourceStorage(std::span<const IResource* const> resources);
		/**
		 * Retrieve the component (read-only) which stores the corresponding opengl backend objects for the resource corresponding to the handle.
		 * @param handle Handle whose resource's component needs to be retrieved. The handle must have referred to one of the initial resources passed to the constructor, otherwise the behaviour is undefined.
		 */
		const IComponent* get_component(ResourceHandle handle) const;
		/**
		 * Retrieve the component which stores the corresponding opengl backend objects for the resource corresponding to the handle.
		 * @param handle Handle whose resource's component needs to be retrieved. The handle must have referred to one of the initial resources passed to the constructor, otherwise the behaviour is undefined.
		 */
		IComponent* get_component(ResourceHandle handle);
		/**
		 * Retrieve the number of resources stored of the given type.
		 * @param type Type whose quantity should be retrieved.
		 * @return Number of resources matching the provided type.
		 */
		unsigned int resource_count_of(ResourceType type) const;
		/**
		 * Bind all buffer resources to their expected resource ids.
		 */
		void bind_buffers();
		/**
		 * Images are converted into bindless texture handles which are then all stored within a secret bespoke SSBO (this does not count as a buffer resource however). This binds that SSBO to the resource id equal to the list of images (this will be equal to the total number of buffer resources).
		 */
		void bind_image_buffer();
		void write_dynamic_images();
		IComponent* try_get_index_buffer() const;
	private:
		void fill_bindless_image_buffer();

		/// Stores components corresponding to each resource.
		std::vector<std::unique_ptr<IComponent>> components;
		/// Stores a bindless texture handle for each image resource.
		std::vector<ogl2::Image::BindlessTextureHandle> image_handles;
		// Shader has an array of texture samplers in tzsl. tzslc compiles this down to actually a storage buffer containing a variable array of texture samplers. This is that buffer.
		ogl2::Buffer bindless_image_storage_buffer;
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
		ShaderManager(const ShaderInfo& sinfo);
		/**
		 * Use the shader program, meaning it will be used in the next draw call.
		 */
		void use();
		bool is_compute() const;
	private:
		ogl2::Shader make_shader(const ShaderInfo& sinfo) const;

		ogl2::Shader shader;
	};

	/**
	 * @ingroup tz_gl2_graphicsapi_ogl_frontend_renderer
	 * Stores information about the output target, and a framebuffer which either points to an offscreen image for render-to-texture (not yet implemented), or the main window framebuffer via ogl2::Framebuffer::null().
	 */
	class OutputManager
	{
	public:
		/**
		 * Create the output target based upon the renderer information.
		 */
		OutputManager(IOutput* output);
		/**
		 * Set this as the render target, causing subsequent draw calls to render into whatever the output is.
		 */
		void set_render_target() const;
	private:
		IOutput* output;
		ogl2::Framebuffer framebuffer;
	};

	using RendererInfoOGL = RendererInfoCommon;

	/**
	 * @ingroup tz_gl2_graphicsapi_ogl_frontend_renderer
	 * Renderer implementation which heavily calls into the backend at @ref tz_gl_ogl2.
	 */
	class RendererOGL
	{
	public:
		/**
		 * Create a new Renderer.
		 * @param info User-exposed class which describes how many resources etc. we have and a high-level description of where we expect to render into.
		 */
		RendererOGL(RendererInfoOGL info);
		// Satisfies RendererType.
		/**
		 * Retrieve the number of resources.
		 */
		unsigned int resource_count() const;
		/**
		 * Retrieve the resource (read-only) corresponding to the given handle.
		 * @param Handle handle returned from a call to a RendererInfoVulkan's `add_resource`. If this handle came from a RendererInfoVulkan different to the one we were provided, the behaviour is undefined.
		 * @return Pointer to the resource.
		 */
		const IResource* get_resource(ResourceHandle handle) const;
		/**
		 * Retrieve the resource corresponding to the given handle.
		 * @param Handle handle returned from a call to a RendererInfoVulkan's `add_resource`. If this handle came from a RendererInfoVulkan different to the one we were provided, the behaviour is undefined.
		 * @return Pointer to the resource.
		 */
		IResource* get_resource(ResourceHandle handle);
		/**
		 * Retrieve the component sourcing the resource (read-only) corresponding to the given handle.
		 * @param Handle handle returned from a call to a RendererInfoVulkan's `add_resource`. If this handle came from a RendererInfoVulkan different to the one we were provided, the behaviour is undefined.
		 * @return Pointer to the resource's underlying component.
		 */
		const IComponent* get_component(ResourceHandle handle) const;
		/**
		 * Retrieve the component sourcing the resource corresponding to the given handle.
		 * @param Handle handle returned from a call to a RendererInfoVulkan's `add_resource`. If this handle came from a RendererInfoVulkan different to the one we were provided, the behaviour is undefined.
		 * @return Pointer to the resource's underlying component.
		 */
		IComponent* get_component(ResourceHandle handle);
		/**
		 * Retrieve options denoting extra features used by the renderer.
		 */
		const RendererOptions& get_options() const;
		/**
		 * Invoke the renderer, emitting a single draw call of a set number of triangles. The number of triangles rendered is equal to the number of triangles rendered in the previous draw-call. If this is the first draw, zero triangles are rendered.
		 */
		void render();
		/**
		 * Invoke the renderer, emitting a single draw call of a set number of triangles.
		 * @param tri_count Number of triangles to render.
		 */
		void render(unsigned int tri_count);
		/**
		 * Confirm changes to a renderer.
		 *
		 * Editing renderers is expensive, so it should only be done if absolutely necessary. If you are editing renderers on a per-frame basis, consider creating multiple different renderers upfront for each hot-path and switching between them as necessary instead.
		 * @param edit_request Structure specifying which edits to make.
		 */
		void edit(const RendererEditRequest& edit_request);
	private:
		ogl2::VertexArray vao;
		ResourceStorage resources;
		ShaderManager shader;
		OutputManager output;
		tz::Vec4 clear_colour;
		tz::Vec3ui compute_kernel;
		RendererOptions options;
		unsigned int tri_count = 0;
	};
	static_assert(RendererType<RendererOGL>);
}

#endif // TZ_OGL
#endif // TOPAZ_GL2_IMPL_FRONTEND_OGL2_RENDERER_HPP
