#ifndef TOPAZ_GL_2_API_DEVICE_HPP
#define TOPAZ_GL_2_API_DEVICE_HPP
#include "gl/declare/image_format.hpp"
#include "gl/api/renderer.hpp"
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
	 * - Have direct access to the window surface (or offline image in headless mode), and exposes relevant information about said surface.
	 */
	template<typename T, typename RendererInfo>
	concept DeviceType = requires(T t, RendererInfo& rinfo)
	{
		requires std::is_default_constructible_v<std::decay_t<T>>;
		requires RendererInfoType<RendererInfo>;

		{t.create_renderer(rinfo)} -> RendererType;
		{t.get_window_format()} -> std::same_as<ImageFormat>;
	};
}

#endif // TOPAZ_GL_2_API_DEVICE_HPP
