#ifndef TOPAZ_GL2_API_RENDERER_HPP
#define TOPAZ_GL2_API_RENDERER_HPP

#include "gl/2/api/input.hpp"
#include "gl/2/api/resource.hpp"
#include "core/containers/basic_list.hpp"
#include <concepts>
namespace tz::gl2
{
	template<typename T>
	concept RendererType = requires(T t, InputHandle i, ResourceHandle r)
	{
		{t.input_count()} -> std::convertible_to<unsigned int>;
		{*(t.get_input(i))} -> std::convertible_to<const IInput*>;

		{t.resource_count()} -> std::convertible_to<unsigned int>;
		{*(t.get_resource(r))} -> std::convertible_to<const IResource*>;

		{t.render()} -> std::same_as<void>;
	};
}

#endif // TOPAZ_GL2_API_RENDERER_HPP
