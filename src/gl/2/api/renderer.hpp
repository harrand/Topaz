#ifndef TOPAZ_GL2_API_RENDERER_HPP
#define TOPAZ_GL2_API_RENDERER_HPP

#include "gl/2/api/input.hpp"
#include "gl/2/api/output.hpp"
#include "gl/2/api/resource.hpp"
#include "gl/2/api/component.hpp"
#include "gl/2/api/shader.hpp"
#include "core/containers/basic_list.hpp"
#include <concepts>
namespace tz::gl2
{
	template<typename T>
	concept RendererInfoType = requires(T t, InputHandle i, ResourceHandle r, IInput& input, IResource& resource, IOutput& output)
	{
		{t.input_count()} -> std::convertible_to<unsigned int>;
		{t.get_input(i)} -> std::convertible_to<const IInput*>;
		{t.get_inputs()} -> std::same_as<std::span<const IInput* const>>;

		{t.resource_count()} -> std::convertible_to<unsigned int>;
		{t.get_resource(r)} -> std::convertible_to<const IResource*>;
		{t.get_resources()} -> std::same_as<std::span<const IResource* const>>;

		{t.add_input(input)} -> std::same_as<InputHandle>;
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
	concept RendererType = requires(T t, InputHandle i, ResourceHandle r)
	{
		{t.input_count()} -> std::convertible_to<unsigned int>;
		{t.get_input(i)} -> std::convertible_to<const IInput*>;

		{t.resource_count()} -> std::convertible_to<unsigned int>;
		{t.get_resource(r)} -> std::convertible_to<const IResource*>;
		{t.get_component(r)} -> std::convertible_to<const IComponent*>;

		{t.render()} -> std::same_as<void>;
	};
}

#endif // TOPAZ_GL2_API_RENDERER_HPP
