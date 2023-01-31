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
		// Final debug ui renderer. Do not use.
		_internal_final_dbgui_renderer,
		// Internal renderer. Has no effect aside from hiding it from the dbgui.
		_internal,
		Count
	};

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

	template<typename T>
	concept renderer_info_type = requires(T t, renderer_handle ren, resource_handle r, const iresource& resource, icomponent* component, ioutput& output, renderer_options options, tz::vec4 vec4, tz::vec3ui vec3ui, std::string str)
	{
		{t.resource_count()} -> std::convertible_to<unsigned int>;
		{t.get_resource(r)} -> std::convertible_to<const iresource*>;
		{t.get_resources()} -> std::same_as<std::vector<const iresource*>>;
		{t.get_dependencies()} -> std::same_as<std::span<const renderer_handle>>;

		{t.add_resource(resource)} -> std::same_as<resource_handle>;
		{t.ref_resource(component)} -> std::same_as<resource_handle>;
		{t.ref_resource(ren, r)} -> std::same_as<resource_handle>;
		{t.set_output(output)} -> std::same_as<void>;
		{t.get_output()} -> std::convertible_to<const ioutput*>;

		{t.set_options(options)} -> std::same_as<void>;
		{t.get_options()} -> std::convertible_to<renderer_options>;

		{t.add_dependency(ren)} -> std::same_as<void>;

		{t.state()} -> std::convertible_to<render_state>;
		{t.shader()} -> shader_info_type;
		{t.debug_name(str)} -> std::same_as<void>;
		{t.debug_get_name()} -> std::same_as<std::string>;
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
