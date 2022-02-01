#ifndef TOPAZ_GL2_API_RENDERER_HPP
#define TOPAZ_GL2_API_RENDERER_HPP

#include "gl/api/output.hpp"
#include "gl/api/resource.hpp"
#include "gl/api/component.hpp"
#include "gl/api/shader.hpp"
#include "core/containers/basic_list.hpp"
#include "core/containers/enum_field.hpp"
#include <concepts>
namespace tz::gl2
{
	/**
	 * @ingroup tz_gl2_renderer
	 * Specifies options to enable extra functionality within Renderers.
	 */
	enum class RendererOption
	{
		/// - Disables depth-testing, with a small gain in performance.
		NoDepthTesting,
	};
	
	using RendererOptions = tz::EnumField<RendererOption>;

	template<typename T>
	concept RendererInfoType = requires(T t, ResourceHandle r, IResource& resource, IOutput& output, RendererOptions options)
	{
		{t.resource_count()} -> std::convertible_to<unsigned int>;
		{t.get_resource(r)} -> std::convertible_to<const IResource*>;
		{t.get_resources()} -> std::same_as<std::span<const IResource* const>>;

		{t.add_resource(resource)} -> std::same_as<ResourceHandle>;
		{t.set_output(output)} -> std::same_as<void>;
		{t.get_output()} -> std::convertible_to<const IOutput*>;

		{t.set_options(options)} -> std::same_as<void>;
		{t.get_options()} -> std::convertible_to<RendererOptions>;

		{t.shader()} -> ShaderInfoType;
	};

	/**
	 * @ingroup tz_gl2_renderer
	 * Named requirement for a Renderer.
	 */
	template<typename T>
	concept RendererType = requires(T t, ResourceHandle r, std::size_t tri_count)
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
	};
}

#endif // TOPAZ_GL2_API_RENDERER_HPP
