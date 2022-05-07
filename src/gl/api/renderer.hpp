#ifndef TOPAZ_GL2_API_RENDERER_HPP
#define TOPAZ_GL2_API_RENDERER_HPP

#include "gl/api/output.hpp"
#include "gl/api/resource.hpp"
#include "gl/api/component.hpp"
#include "gl/api/shader.hpp"
#include "core/containers/basic_list.hpp"
#include "core/containers/enum_field.hpp"
#include "core/vector.hpp"
#include <concepts>
#include <variant>

namespace tz::gl
{
	/**
	 * @ingroup tz_gl2_renderer
	 * Specifies options to enable extra functionality within Renderers.
	 */
	enum class RendererOption
	{
		/// - Disables depth-testing, with a small gain in performance.
		NoDepthTesting,
		/// - Enables alpha-blending. Causes pixels with alpha value <1.0 to blend with the previous colour in the framebuffer, at a small cost to performance.
		AlphaBlending,
		/// - When a compute renderer's `render()` is invoked, the thread will block until the compute-shader has finished processing.
		BlockingCompute
	};
	
	/**
	 * @ingroup tz_gl2_renderer
	 * Represents a collection of renderer options.
	 */
	using RendererOptions = tz::EnumField<RendererOption>;

	template<typename T>
	concept RendererInfoType = requires(T t, ResourceHandle r, IResource& resource, IOutput& output, RendererOptions options, tz::Vec4 vec4, tz::Vec3ui vec3ui)
	{
		{t.resource_count()} -> std::convertible_to<unsigned int>;
		{t.get_resource(r)} -> std::convertible_to<const IResource*>;
		{t.get_resources()} -> std::same_as<std::span<const IResource* const>>;

		{t.add_resource(resource)} -> std::same_as<ResourceHandle>;
		{t.set_output(output)} -> std::same_as<void>;
		{t.get_output()} -> std::convertible_to<const IOutput*>;

		{t.set_options(options)} -> std::same_as<void>;
		{t.get_options()} -> std::convertible_to<RendererOptions>;

		{t.set_clear_colour(vec4)} -> std::same_as<void>;
		{t.get_clear_colour()} -> std::convertible_to<tz::Vec4>;

		{t.set_compute_kernel(vec3ui)} -> std::same_as<void>;
		{t.get_compute_kernel()} -> std::convertible_to<tz::Vec3ui>;

		{t.shader()} -> ShaderInfoType;
	};

	/**
	 * Represents an edit to an existing buffer component owned by an existing renderer.
	 */
	struct RendererBufferComponentEditRequest
	{
		/// Handle corresponding to the buffer to edit.
		ResourceHandle buffer_handle;
		/// New size of the buffer, in bytes.
		std::size_t size;
	};

	/**
	 * Represents an edit to an existing image component owned by an existing renderer.
	 */
	struct RendererImageComponentEditRequest
	{
		/// Handle corresponding to the image to edit.
		ResourceHandle image_handle;
		/// New dimensions of the image, in pixels.
		tz::Vec2ui dimensions;
	};

	/**
	 * Represents an edit, setting a new value for the compute kernel for a renderer.
	 */
	struct RendererComputeEditRequest
	{
		/// New compute kernel workgroup dimensions.
		tz::Vec3ui kernel;
	};

	/**
	 * Type-safe union of @ref RendererBufferComponentEditRequest and @ref RendererImageComponentEditRequest
	 */
	using RendererComponentEditRequest = std::variant<RendererBufferComponentEditRequest, RendererImageComponentEditRequest>;

	/**
	 * @ingroup tz_gl2_renderer
	 * Represents an edit to an existing renderer.
	 */
	struct RendererEditRequest
	{
		/**
		 * List of edits to components owned by the renderer.
		 */
		std::vector<RendererComponentEditRequest> component_edits;
		std::optional<RendererComputeEditRequest> compute_edit = std::nullopt;
	};

	/**
	 * @ingroup tz_gl2_renderer
	 * Named requirement for a Renderer.
	 */
	template<typename T>
	concept RendererType = requires(T t, ResourceHandle r, std::size_t tri_count, const RendererEditRequest& edit_request)
	{
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
	};
}

#endif // TOPAZ_GL2_API_RENDERER_HPP
