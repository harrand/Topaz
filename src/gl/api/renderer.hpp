#ifndef TOPAZ_GL2_API_RENDERER_HPP
#define TOPAZ_GL2_API_RENDERER_HPP

#include "gl/api/output.hpp"
#include "gl/api/resource.hpp"
#include "gl/api/component.hpp"
#include "gl/api/shader.hpp"
#include "core/containers/basic_list.hpp"
#include <concepts>
namespace tz::gl2
{
	template<typename T>
	concept RendererInfoType = requires(T t, ResourceHandle r, IResource& resource, IOutput& output)
	{
		{t.resource_count()} -> std::convertible_to<unsigned int>;
		{t.get_resource(r)} -> std::convertible_to<const IResource*>;
		{t.get_resources()} -> std::same_as<std::span<const IResource* const>>;

		{t.add_resource(resource)} -> std::same_as<ResourceHandle>;
		{t.set_output(output)} -> std::same_as<void>;
		{t.get_output()} -> std::convertible_to<const IOutput*>;

		{t.shader()} -> ShaderInfoType;
	};

	/**
	 * @ingroup tz_gl2
	 * Named requirement for a Renderer.
	 */
	template<typename T>
	concept RendererType = requires(T t, ResourceHandle r, std::size_t tri_count)
	{
		{t.resource_count()} -> std::convertible_to<unsigned int>;
		{t.get_resource(r)} -> std::convertible_to<const IResource*>;
		{t.get_component(r)} -> std::convertible_to<const IComponent*>;

		{t.render()} -> std::same_as<void>;
		{t.render(tri_count)} -> std::same_as<void>;
	};
}

#endif // TOPAZ_GL2_API_RENDERER_HPP
