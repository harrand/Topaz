#ifndef TOPAZ_GL_2_API_DEVICE_HPP
#define TOPAZ_GL_2_API_DEVICE_HPP
#include "tz/gl/declare/image_format.hpp"
#include "tz/gl/api/renderer.hpp"
#include <functional>
#include <type_traits>

namespace tz::gl
{
	/**
	 * @ingroup tz_gl2
	 * Named Requirement: device
	 *
	 * device types are types which:
	 * - Implement the factory pattern for various graphics library objects, such as @ref renderer_type
	 * - Have direct access to the window surface, and exposes relevant information about said surface.
	 */
	template<typename T, typename R>
	concept device_type = requires(T t, R& rinfo, renderer_handle h)
	{
		requires std::is_default_constructible_v<std::decay_t<T>>;
		requires renderer_info_type<R>;

		{t.create_renderer(rinfo)} -> std::same_as<renderer_handle>;
		{t.destroy_renderer(h)} -> std::same_as<void>;
		{t.renderer_count()} -> std::convertible_to<std::size_t>;
		{t.get_renderer(h)} -> renderer_type;
		{t.get_window_format()} -> std::same_as<image_format>;
		{t.dbgui()} -> std::same_as<void>;
	};
}

#endif // TOPAZ_GL_2_API_DEVICE_HPP
