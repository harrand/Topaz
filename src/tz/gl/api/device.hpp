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
	 * Named Requirement: Device
	 *
	 * Device types are types which:
	 * - Implement the factory pattern for various graphics library objects, such as @ref RendererType
	 * - Have direct access to the window surface, and exposes relevant information about said surface.
	 */
	template<typename T, typename RendererInfo>
	concept DeviceType = requires(T t, RendererInfo& rinfo, RendererHandle h)
	{
		requires std::is_default_constructible_v<std::decay_t<T>>;
		requires RendererInfoType<RendererInfo>;

		{t.create_renderer(rinfo)} -> std::same_as<RendererHandle>;
		{t.get_renderer(h)} -> RendererType;
		{t.get_window_format()} -> std::same_as<ImageFormat>;
	};
}

#endif // TOPAZ_GL_2_API_DEVICE_HPP
