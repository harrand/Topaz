#ifndef TOPAZ_GL2_API_RENDERER_HPP
#define TOPAZ_GL2_API_RENDERER_HPP

#include "tz/core/data/handle.hpp"
#include "tz/gl/api/output.hpp"
#include "tz/gl/api/resource.hpp"
#include "tz/gl/api/component.hpp"
#include "tz/gl/api/shader.hpp"
#include "tz/core/data/basic_list.hpp"
#include "tz/core/data/enum_field.hpp"
#include "tz/core/data/vector.hpp"
#include <concepts>
#include <variant>
#include <optional>

namespace tz::gl
{
	namespace detail
	{
		struct renderer_tag{};
	}

	/**
	 * @ingroup tz_gl2_renderer
	 * Represents a handle for a renderer owned by an existing device.
	 */
	using renderer_handle = tz::handle<detail::renderer_tag>;

	/**
	 * @ingroup tz_gl2_renderer
	 * Specifies options to enable extra functionality within Renderers.
	 */
	enum class renderer_option
	{
		/// - Disables depth-testing and depth-writing.
		no_depth_testing,
		/// - Enables alpha-blending. Causes pixels with alpha value <1.0 to blend with the previous colour in the framebuffer, at a small cost to performance.
		alpha_blending,
		/// - When a compute renderer's `render()` is invoked, the thread will block until the compute-shader has finished processing.
		render_wait,
		/// - When a renderer is invoked, the output image is not cleared before being rendered to. If nothing has been rendered into the output this frame, then the behaviour is undefined.
		no_clear_output,
		/// - When a renderer is invoked, the output image is not presented to the screen. If the output is an image_output, this has no effect.
		no_present,
		/// - The renderer's `state().graphics.draw_buffer` is assumed to contain a uint32 representing the draw count at the very start of the buffer, and the draw commands appear after that count in memory.
		draw_indirect_count,
		// Final debug ui renderer. Do not use.
		_internal_final_dbgui_renderer,
		// Internal renderer. Has no effect aside from hiding it from the dbgui.
		_internal,
		Count
	};
	namespace detail
	{
		constexpr std::array<const char*, static_cast<int>(tz::gl::renderer_option::Count)> renderer_option_strings =
		{
			"No Depth Testing",
			"Alpha Blending",
			"Render Wait",
			"No Clear Output",
			"No Present",
			"Final Debug UI renderer (Internal)",
			"Internal",
		};
	}

	/**
	 * @ingroup tz_gl2_renderer
	 * Stores renderer-specific state, which drives the behaviour of the rendering.
	 */
	struct render_state
	{
		struct Graphics
		{
			/// If an index buffer is used, this refers to the buffer resource. It must have resource_flag::index_buffer. If there is no index buffer, this is nullhand.
			resource_handle index_buffer = tz::nullhand;
			/// If a draw-indirect buffer is used, this refers to the buffer resource. It must have resource_flag::draw_indirect_buffer. If there is no draw buffer, this is nullhand.
			resource_handle draw_buffer = tz::nullhand;
			/// Normalised RGBA floating point colour.
			tz::vec4 clear_colour = tz::vec4::zero();
			/// number of triangles to be rendered in the next draw call.
			std::size_t tri_count = 0;
			bool operator==(const Graphics& rhs) const = default;
		};
		struct Compute
		{
			/// Represents the compute kernel, which contains the number of workgroups to be launched in the X, Y and Z dimensions.
			tz::vec3ui kernel = {1u, 1u, 1u};
			bool operator==(const Compute& rhs) const = default;
		};
		/// Graphics state.
		Graphics graphics;
		/// Compute state.
		Compute compute;
		
		bool operator==(const render_state& rhs) const = default;
	};
	
	/**
	 * @ingroup tz_gl2_renderer
	 * Represents a collection of renderer options.
	 */
	using renderer_options = tz::enum_field<renderer_option>;

	/**
	 * @ingroup tz_gl2_renderer
	 * Helper struct which the user can use to specify which inputs, resources they want and where they want a renderer to render to. This is likely going to be refactored at some point because it doesn't do any Vulkan-specific magic.
	 */
	class renderer_info
	{
	public:
		renderer_info();
		// Satisfies renderer_info_type.
		/**
		 * Retrieve the number of resources.
		 */
		unsigned int resource_count() const;
		/**
		 * Retrieve the resource corresponding to the given handle.
		 * @param Handle handle returned from a previous call to `add_resource`. If this handle came from a different renderer_info, the behaviour is undefined.
		 * @return Pointer to the resource.
		 */
		const iresource* get_resource(resource_handle handle);
		/**
		 * Retrieve a span containing all of the specified resources. Size of the span is guaranteed to be equal to @ref resource_count()
		 */
		std::vector<const iresource*> get_resources() const;
		std::span<const renderer_handle> get_dependencies() const;
		std::span<const icomponent* const> get_components() const;
		/**
		 * Add a new resource, which will be used by a renderer which is created from this helper struct.
		 *
		 * @param resource resource which will be owned by a renderer.
		 * @return Handle corresponding to the resource. If you want to retrieve the resource later, you should keep ahold of this handle.
		 */
		resource_handle add_resource(const iresource& resource);
		/**
		 * Deprecated. See @ref ref_resource(renderer_handle, resource_handle)
		 */
		resource_handle ref_resource(icomponent* component);
		/**
		 * Adds a resource to this renderer which already exists and is owned by another renderer.
		 * @param ren Handle associated with the existing renderer that owns the resource.
		 * @param res Handle associated with the resource that should be referenced.
		 */
		resource_handle ref_resource(renderer_handle ren, resource_handle res);
		/**
		 * Renderers always render into something. By default, it renders to the window (only one window is supported so no confusion there). You can however set it to render into something else, such as a @ref TextureOutput if you want to render into the resource of another renderer.
		 */
		void set_output(const ioutput& output);
		/**
		 * Retrieve the current render output (read-only). This may return nullptr, meaning that the main window will be rendered into.
		 */
		const ioutput* get_output() const;
		/**
		 * Retrieve the currently specified options which will be used by the renderer.
		 */
		const renderer_options& get_options() const;
		/**
		 * Set the currently specified options which will be used by the renderer.
		 */
		void set_options(renderer_options options);
		/**
		 * Set the pending renderer to be dependent on the specified renderer.
		 *
		 * This means that when render() is invoked, the GPU will wait on completion of render-work of the specified renderer before the render work of this renderer begins. This also means that the specified renderer *must* run each time this is ran, ahead-of-time.
		 */
		void add_dependency(renderer_handle dependency);
		/**
		 * Read/write information about the state of the renderer when it is created.
		 */
		render_state& state();
		/**
		 * Read-only information about the state of the renderer when it is created.
		 */
		const render_state& state() const;
		/**
		 * Read/write information about the shader that will be built for the renderer.
		 */
		shader_info& shader();
		/**
		 * Read-only information about the shader that will be built for the renderer.
		 */
		const shader_info& shader() const;
		/**
		 * Set the debug name of the spawned renderer. By default, the debug name is a compact description of the renderer.
		 * @note This only has an affect on debug builds.
		 */
		void debug_name(std::string debug_name);
		/**
		 * Retrieve the debug name which will be used for the spawned renderer.
		 */
		std::string debug_get_name() const;
	private:
		std::size_t real_resource_count() const;
		/// Stores all provided resources. It is assumed that their lifetime is valid for the entirety of this helper struct's lifetime.
		std::vector<std::unique_ptr<iresource>> resources = {};
		/// Stores all provided components. In this context, components act as references to existing resources owned by another renderer.
		std::vector<icomponent*> components = {};
		/// Output. Can be null, which defaults to rendering into the main window.
		std::unique_ptr<ioutput> output = nullptr;
		/// Specifies which extra features the renderer will have.
		renderer_options options = {};
		/// Describes render state. It could change.
		render_state renderer_state = {};
		/// List of renderers we are dependent on.
		std::vector<renderer_handle> dependencies = {};
		/// Describes the shader sources used.
		shader_info shader_info;
		/// The clear value for colour attachments.
		tz::vec4 clear_colour = {0.0f, 0.0f, 0.0f, 1.0f};
		/// Workgroup sizes if we're doing compute.
		tz::vec3ui compute_kernel = {1u, 1u, 1u};
		std::string dbg_name = "";
	};

	struct renderer_edit
	{
		/**
		 * Represents a resize operation for an existing buffer component.
		 */
		struct buffer_resize
		{
			/// Handle corresponding to the buffer to edit.
			resource_handle buffer_handle;
			/// New size of the buffer, in bytes.
			std::size_t size;
		};

		/**
		 * Represents a resize operation for an existing image component.
		 */
		struct image_resize
		{
			/// Handle corresponding to the image to edit.
			resource_handle image_handle;
			/// New dimensions of the image, in pixels.
			tz::vec2ui dimensions;
		};

		struct resource_write
		{
			resource_handle resource;
			std::span<const std::byte> data = {};
			std::size_t offset = 0;
		};

		struct resource_reference
		{
			resource_handle resource;
			icomponent* component;
		};

		/**
		 * Represents an edit, setting a new value for the compute kernel for a renderer.
		 */
		struct compute_config
		{
			/// New compute kernel workgroup dimensions.
			tz::vec3ui kernel;
		};

		/**
		 * Represents an edit to the fixed-function renderer state.
		 */
		struct render_config
		{
			/// Whether triangles should only have their outlines drawn, instead of filled.
			bool wireframe_mode = false;
		};

		using variant = std::variant<buffer_resize, image_resize, resource_write, resource_reference, compute_config, render_config>;
	};

	/**
	 * @ingroup tz_gl2_renderer
	 * Represents an edit to an existing renderer.
	 *
	 * @note This is a large structure. You should use the helper class @ref RendererEditBuilder to create one of these instead of attempting to fill it directly.
	 */
	using renderer_edit_request = std::vector<renderer_edit::variant>;

	/**
	 * @ingroup tz_gl2_renderer
	 * Helper class which can be used to generate a @ref renderer_edit_request.
	 */
	class RendererEditBuilder
	{
	public:
		RendererEditBuilder() = default;
		/**
		 * Make amendments to the compute configuration of the renderer. See @ref RendererComputeEditRequest for details.
		 */
		RendererEditBuilder& compute(renderer_edit::compute_config req);
		/**
		 * Make amendments to the current render state. See @ref RenderStateEditRequest for details.
		 */
		RendererEditBuilder& render_state(renderer_edit::render_config req);
		/**
		 * Make amendments to an existing image resource. See @ref RendererImageComponentResizeRequest for details.
		 */
		RendererEditBuilder& image_resize(renderer_edit::image_resize req);
		/**
		 * Make amendments to an existing buffer resource. See @ref RendererBufferComponentResizeRequest for details.
		 */
		RendererEditBuilder& buffer_resize(renderer_edit::buffer_resize req);
		RendererEditBuilder& write(renderer_edit::resource_write req);
		/**
		 * Retrieve a @ref renderer_edit_request corresponding to all edits specified within the builder so far.
		 */
		renderer_edit_request build() const;
	private:
		renderer_edit_request request = {};
	};

	/**
	 * @ingroup tz_gl2_renderer
	 * Named requirement for a renderer.
	 */
	template<typename T>
	concept renderer_type = requires(T t, resource_handle r, std::size_t tri_count, const renderer_edit_request& edit_request)
	{
		requires tz::nullable<T>;
		/**
		 * Retrieves the number of resources used by the renderer.
		 * @return number of resources of any type.
		 */
		{t.resource_count()} -> std::convertible_to<unsigned int>;
		/**
		 * Retrieve an existing renderer resource via an existing handle.
		 * @param r Handle corresponding to an existing resource. If the handle is not associated with this renderer, the behaviour is undefined.
		 * @return Pointer to the resource corresponding to the provided handle.
		 */
		{t.get_resource(r)} -> std::convertible_to<const iresource*>;
		/**
		 * Retrieve an existing renderer component via an existing handle.
		 * @param r Handle corresponding to an existing resource. If the handle is not associated with this renderer, the behaviour is undefined.
		 * @return Pointer to the component based upon the resource corresponding to the provided handle.
		 */
		{t.get_component(r)} -> std::convertible_to<const icomponent*>;
		/**
		 * Retrieve the renderer output, if there is one.
		 *
		 * @note This may return nullptr, in which case it is rendering directly into the window with no custom viewport/scissor.
		 */
		{t.get_output()} -> std::convertible_to<const ioutput*>;
		/**
		 * Retrieve the options with which the renderer was constructed.
		 * @return Options containing additional features used by the renderer.
		 */
		{t.get_options()} -> std::convertible_to<renderer_options>;
		{t.get_state()} -> std::convertible_to<render_state>;
		/**
		 * Invoke the renderer, emitting a single draw call of a set number of triangles. The number of triangles renderered is equal to the number of triangles rendered in the previous draw-call. If this is the first draw, zero triangles are rendered.
		 */
		{t.render()} -> std::same_as<void>;
		/**
		 * Invoke the renderer, emitting a single draw call of a set number of triangles.
		 * @param tri_count number of triangles to render.
		 */
		{t.render(tri_count)} -> std::same_as<void>;
		/**
		 * Confirm changes to a renderer.
		 *
		 * Editing renderers is expensive, so it should only be done if absolutely necessary. If you are editing renderers on a per-frame basis, consider creating multiple different renderers upfront for each hot-path and switching between them as necessary instead.
		 * @param edit_request Structure specifying which edits to make.
		 */
		{t.edit(edit_request)} -> std::same_as<void>;
		/**
		 * Display information about the renderer in the debug ui.
		 */
		{t.dbgui()} -> std::same_as<void>;
		{t.debug_get_name()} -> std::convertible_to<std::string_view>;
	};

	#if TZ_VULKAN && TZ_OGL
		// Documentation only.
		/**
		 * @ingroup tz_gl2_renderer
		 * Implements @ref tz::gl::renderer_type
		 */
		class renderer
		{
		public:
			/**
			 * Retrieves the number of resources used by the renderer.
		 	 */
			unsigned int resource_count() const;	
			/**
			 * Retrieves a pointer to the resource via its associated handle.
			 * @pre The handle `rh` must have been retrieved from a prior call to @ref tz::gl::renderer_info::add_resource() or @ref tz::gl::renderer_info::ref_resource(), where the @ref tz::gl::renderer_info was later used by @ref tz::gl::device::create_renderer() to create the current renderer object. Otherwise, the behaviour is undefined.
			 * @param rh Handle corresponding to the resource.
			 * @return Pointer to the associated resource, or nullptr if no such resource exists.
			 */ 
			const iresource* get_resource(tz::gl::resource_handle rh) const;
			/**
			 * Retrieves a pointer to the resource via its associated handle.
			 * @pre The handle `rh` must have been retrieved from a prior call to @ref tz::gl::renderer_info::add_resource() or @ref tz::gl::renderer_info::ref_resource(), where the @ref tz::gl::renderer_info was later used by @ref tz::gl::device::create_renderer() to create the current renderer object. Otherwise, the behaviour is undefined.
			 * @param rh Handle corresponding to the resource.
			 * @return Pointer to the associated resource, or nullptr if no such resource exists.
			 */ 
			iresource* get_resource(tz::gl::resource_handle rh);

			/**
			 * Retrieve the output that this renderer draws to.
			 * @return Pointer to the output, or `nullptr` if there is no output (Meaning the renderer draws directly to the window).
			 */
			const ioutput* get_output() const;
			/**
			 * Retrieve the output that this renderer draws to.
			 * @return Pointer to the output, or `nullptr` if there is no output (Meaning the renderer draws directly to the window).
			 */
			ioutput* get_output();

			/**
			 * Retrieve the set of enabled @ref tz::gl::renderer_option flags used by this renderer.
			 * @return Set of options used by the renderer.
			 */ 
			tz::gl::renderer_options get_options() const;
			/**
			 * Retrieve the current render state.
			 * @return Information about the renderer's current state.
			 */ 
			const tz::gl::render_state& get_state() const;
			/**
			 * Submit the renderer's work to the GPU.
			 * 
			 * This function returns instantly.
			 * - To have this function block the current thread until the work is complete, see @ref tz::gl::renderer_option::render_wait.
			 * - There is no way to query whether the GPU work is complete after-the-fact. However, you may be interested in @ref renderer_dependencies (WIP).
			 *
			 * If this is a graphics renderer, the number of triangles submitted in the draw call will be equal to the most recently provided triangle count in a previous invocation to @ref tz::gl::renderer::render(std::size_t). If this method was never invoked on this renderer object, a triangle count of `0` will be used.
			 */ 
			void render();
			/**
			 * Submit the renderer's work to the GPU.
			 *
			 * This function returns instantly.
			 * - To have this function block the current thread until the work is complete, see @ref tz::gl::renderer_option::render_wait.
			 * - There is no way to query whether the GPU work is complete after-the-fact. However, you may be interested in @ref renderer_dependencies (WIP).
			 *
			 * @param tri_count The number of triangles that will be drawn by the renderer.
			 * 
			 * - If the renderer has an index buffer, `tri_count` refers instead to the number of indices that will be drawn by the renderer.
			 * - If the renderer has a draw-indirect buffer, `tri_count` is ignored.
			 * - If the renderer is a compute renderer, `tri_count` is ignored.
			 */
			void render(std::size_t tri_count);
		};
	#endif
}

#endif // TOPAZ_GL2_API_RENDERER_HPP
