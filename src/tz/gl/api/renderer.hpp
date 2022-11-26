#ifndef TOPAZ_GL2_API_RENDERER_HPP
#define TOPAZ_GL2_API_RENDERER_HPP

#include "hdk/data/handle.hpp"
#include "tz/gl/api/output.hpp"
#include "tz/gl/api/resource.hpp"
#include "tz/gl/api/component.hpp"
#include "tz/gl/api/shader.hpp"
#include "tz/core/containers/basic_list.hpp"
#include "tz/core/containers/enum_field.hpp"
#include "hdk/data/vector.hpp"
#include <concepts>
#include <variant>
#include <optional>

namespace tz::gl
{
	namespace detail
	{
		struct RendererTag{};
	}

	/**
	 * @ingroup tz_gl2_renderer
	 * Represents a handle for a renderer owned by an existing device.
	 */
	using RendererHandle = hdk::handle<detail::RendererTag>;

	/**
	 * @ingroup tz_gl2_renderer
	 * Specifies options to enable extra functionality within Renderers.
	 */
	enum class RendererOption
	{
		/// - Disables depth-testing and depth-writing.
		NoDepthTesting,
		/// - Enables alpha-blending. Causes pixels with alpha value <1.0 to blend with the previous colour in the framebuffer, at a small cost to performance.
		AlphaBlending,
		/// - When a compute renderer's `render()` is invoked, the thread will block until the compute-shader has finished processing.
		RenderWait,
		/// - When a renderer is invoked, the output image is not cleared before being rendered to. If nothing has been rendered into the output this frame, then the behaviour is undefined.
		NoClearOutput,
		/// - When a renderer is invoked, the output image is not presented to the screen. If the output is an ImageOutput, this has no effect.
		NoPresent,
		// Final debug ui renderer. Do not use.
		Internal_FinalDebugUIRenderer,
		// Internal renderer. Has no effect aside from hiding it from the dbgui.
		Internal,
		Count
	};
	
	/**
	 * @ingroup tz_gl2_renderer
	 * Represents a collection of renderer options.
	 */
	using RendererOptions = tz::EnumField<RendererOption>;

	template<typename T>
	concept RendererInfoType = requires(T t, RendererHandle ren, ResourceHandle r, const IResource& resource, IComponent* component, IOutput& output, RendererOptions options, hdk::vec4 vec4, hdk::vec3ui vec3ui, std::string str)
	{
		{t.resource_count()} -> std::convertible_to<unsigned int>;
		{t.get_resource(r)} -> std::convertible_to<const IResource*>;
		{t.get_resources()} -> std::same_as<std::vector<const IResource*>>;

		{t.add_resource(resource)} -> std::same_as<ResourceHandle>;
		{t.ref_resource(component)} -> std::same_as<ResourceHandle>;
		{t.ref_resource(ren, r)} -> std::same_as<ResourceHandle>;
		{t.set_output(output)} -> std::same_as<void>;
		{t.get_output()} -> std::convertible_to<const IOutput*>;

		{t.set_options(options)} -> std::same_as<void>;
		{t.get_options()} -> std::convertible_to<RendererOptions>;

		{t.set_clear_colour(vec4)} -> std::same_as<void>;
		{t.get_clear_colour()} -> std::convertible_to<hdk::vec4>;

		{t.set_compute_kernel(vec3ui)} -> std::same_as<void>;
		{t.get_compute_kernel()} -> std::convertible_to<hdk::vec3ui>;

		{t.shader()} -> ShaderInfoType;
		{t.debug_name(str)} -> std::same_as<void>;
		{t.debug_get_name()} -> std::same_as<std::string>;
	};

	struct RendererEdit
	{

		/**
		 * Represents a resize operation for an existing buffer component.
		 */
		struct BufferResize
		{
			/// Handle corresponding to the buffer to edit.
			ResourceHandle buffer_handle;
			/// New size of the buffer, in bytes.
			std::size_t size;
		};

		/**
		 * Represents a resize operation for an existing image component.
		 */
		struct ImageResize
		{
			/// Handle corresponding to the image to edit.
			ResourceHandle image_handle;
			/// New dimensions of the image, in pixels.
			hdk::vec2ui dimensions;
		};

		struct ResourceWrite
		{
			ResourceHandle resource;
			std::span<const std::byte> data = {};
			std::size_t offset = 0;
		};

		struct ResourceReference
		{
			ResourceHandle resource;
			IComponent* component;
		};

		/**
		 * Represents an edit, setting a new value for the compute kernel for a renderer.
		 */
		struct ComputeConfig
		{
			/// New compute kernel workgroup dimensions.
			hdk::vec3ui kernel;
		};

		/**
		 * Represents an edit to the fixed-function renderer state.
		 */
		struct RenderConfig
		{
			/// Whether triangles should only have their outlines drawn, instead of filled.
			bool wireframe_mode = false;
		};

		using Variant = std::variant<BufferResize, ImageResize, ResourceWrite, ResourceReference, ComputeConfig, RenderConfig>;
	};

	/**
	 * @ingroup tz_gl2_renderer
	 * Represents an edit to an existing renderer.
	 *
	 * @note This is a large structure. You should use the helper class @ref RendererEditBuilder to create one of these instead of attempting to fill it directly.
	 */
	using RendererEditRequest = std::vector<RendererEdit::Variant>;

	/**
	 * @ingroup tz_gl2_renderer
	 * Named requirement for a Renderer.
	 */
	template<typename T>
	concept RendererType = requires(T t, ResourceHandle r, std::size_t tri_count, const RendererEditRequest& edit_request)
	{
		requires tz::Nullable<T>;
		/**
		 * Retrieves the number of resources used by the renderer.
		 * @return Number of resources of any type.
		 */
		{t.resource_count()} -> std::convertible_to<unsigned int>;
		/**
		 * Retrieve an existing renderer resource via an existing handle.
		 * @param r Handle corresponding to an existing resource. If the handle is not associated with this renderer, the behaviour is undefined.
		 * @return Pointer to the resource corresponding to the provided handle.
		 */
		{t.get_resource(r)} -> std::convertible_to<const IResource*>;
		/**
		 * Retrieve an existing renderer component via an existing handle.
		 * @param r Handle corresponding to an existing resource. If the handle is not associated with this renderer, the behaviour is undefined.
		 * @return Pointer to the component based upon the resource corresponding to the provided handle.
		 */
		{t.get_component(r)} -> std::convertible_to<const IComponent*>;
		/**
		 * Retrieve the renderer output, if there is one.
		 *
		 * @note This may return nullptr, in which case it is rendering directly into the window with no custom viewport/scissor.
		 */
		{t.get_output()} -> std::convertible_to<const IOutput*>;
		/**
		 * Retrieve the options with which the renderer was constructed.
		 * @return Options containing additional features used by the renderer.
		 */
		{t.get_options()} -> std::convertible_to<RendererOptions>;
		/**
		 * Invoke the renderer, emitting a single draw call of a set number of triangles. The number of triangles renderered is equal to the number of triangles rendered in the previous draw-call. If this is the first draw, zero triangles are rendered.
		 */
		{t.render()} -> std::same_as<void>;
		/**
		 * Invoke the renderer, emitting a single draw call of a set number of triangles.
		 * @param tri_count Number of triangles to render.
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
}

#endif // TOPAZ_GL2_API_RENDERER_HPP
