#ifndef TZ_WSI_API_WINDOW_HPP
#define TZ_WSI_API_WINDOW_HPP
#include "tz/wsi/api/keyboard.hpp"
#include "tz/wsi/api/mouse.hpp"
#include "hdk/data/vector.hpp"
#if TZ_VULKAN
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(__linux__)
#define VK_USE_PLATFORM_XLIB_KHR
#else
static_assert("Platform not supported for a vulkan build.");
#endif
#include <vulkan/vulkan.h>
#endif // TZ_VULKAN

namespace tz::wsi
{
	/**
	 * @ingroup tz_wsi_window
	 * Represents an optional setting for a `tz::wsi::window`. For default behaviour, refer to @ref window_flag::none
	 */
	namespace window_flag
	{
		using flag_bit = int;
		static constexpr flag_bit
		/// Normal window, no special flags.
		none = 0x00,
		/// Window will create and maintain its own OpenGL context. During the lifetime of a created window, it is valid to call OpenGL functions (OpenGL desktop 4.5).
		opengl = 0x01 << 0,
		/// Window will have a transparent background.
		transparent = 0x01 << 1,
		/// Window will have a smaller title bar, without a maximise/minimise/close button.
		undecorated = 0x01 << 2,
		/// Window will have no title bar at all.
		bare = 0x01 << 3,
		/// Window is not resizeable.
		noresize = 0x01 << 4,
		/// Window cannot be maximised.
		nomaximise = 0x01 << 5,
		/// Window cannot be minimised.
		nominimise = 0x01 << 6,
		/// Window is invisible.
		invisible = 0x01 << 7;
	}

	/**
	 * @ingroup tz_wsi_window
	 * Specifies creation flags for a `wsi::window`.
	 */
	struct window_info
	{
		/// Title of the window. Default is "Untitled".
		const char* title = "Untitled";
		/// Dimensions of the window, in pixels. Default is 800x600.
		hdk::vec2ui dimensions = {800u, 600u};
		/// Extra behaviour for the window. Default is none.
		window_flag::flag_bit window_flags = window_flag::none;
	};

	/**
	 * @ingroup tz_wsi_window
	 * Represents an API for a window. `wsi::window` is implemented separately per platform, but all implementations are guaranteed to satisfy this api.
	 */
	template<typename T>
	concept window_api = requires(T t, hdk::vec2ui dims, void* addr, std::string str
	#if TZ_VULKAN
	, VkInstance vkinst
	#endif
	)
	{
		typename T::native;
		{t.get_native()} -> std::convertible_to<typename T::native>;
		{t.is_close_requested()} -> std::same_as<bool>;
		{t.get_dimensions()} -> std::same_as<hdk::vec2ui>;
		{t.set_dimensions(dims)} -> std::same_as<void>;
		{t.get_title()} -> std::convertible_to<std::string>;
		{t.set_title(str)} -> std::same_as<void>;
		{t.get_flags()} -> std::convertible_to<window_flag::flag_bit>;
		{t.update()} -> std::same_as<void>;
		{t.make_opengl_context_current()} -> std::same_as<bool>;
		#if TZ_VULKAN
		{t.make_vulkan_surface(vkinst)} -> std::convertible_to<VkSurfaceKHR>;
		#endif // TZ_VULKAN
		{t.get_keyboard_state()} -> std::convertible_to<keyboard_state>;
		{t.get_mouse_state()} -> std::convertible_to<mouse_state>;
		{t.get_user_data()} -> std::convertible_to<void*>;
		{t.set_user_data(addr)} -> std::same_as<void>;
	};
}

#endif // TZ_WSI_API_WINDOW_HPP
