#ifndef TANGLE_API_WINDOW_HPP
#define TANGLE_API_WINDOW_HPP
#include "tz/wsi/api/keyboard.hpp"
#include "tz/wsi/api/mouse.hpp"
#include "hdk/data/vector.hpp"

namespace tz::wsi
{
	/**
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
		nominimise = 0x01 << 6;
	}
	struct window_info
	{
		const char* title = "Untitled";
		hdk::vec2ui dimensions = {800u, 600u};
		window_flag::flag_bit window_flags = window_flag::none;
	};

	template<typename T>
	concept window_api = requires(T t, hdk::vec2ui dims, void* addr, std::string str)
	{
		typename T::native;
		{t.get_native()} -> std::convertible_to<typename T::native>;
		{t.is_close_requested()} -> std::same_as<bool>;
		{t.get_dimensions()} -> std::same_as<hdk::vec2ui>;
		{t.set_dimensions(dims)} -> std::same_as<void>;
		{t.get_title()} -> std::convertible_to<std::string>;
		{t.set_title(str)} -> std::same_as<void>;
		{t.update()} -> std::same_as<void>;
		{t.make_opengl_context_current()} -> std::same_as<bool>;
		{t.get_keyboard_state()} -> std::convertible_to<keyboard_state>;
		{t.get_mouse_state()} -> std::convertible_to<mouse_state>;
		{t.get_user_data()} -> std::convertible_to<void*>;
		{t.set_user_data(addr)} -> std::same_as<void>;
	};
}

#endif // TANGLE_API_WINDOW_HPP
