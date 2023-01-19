#ifndef TZ_WSI_IMPL_LINUX_WINDOW_HPP
#define TZ_WSI_IMPL_LINUX_WINDOW_HPP
#ifdef __linux__
#include "tz/wsi/api/window.hpp"
#include "tz/wsi/impl/linux/wsi_linux.hpp"
// glx includes gl.h, and glad.h does aswell, but will complain if gl.h is already included elsewhere aside from glad.h. For this reason, we include glad.h even though we don't need it, so glx.h has access to ogl types without defining it itself. Now we can go on to include glad elsewhere in another including TU without errors.
#include "glad/glad.h"
#include <GL/glx.h>
#include <optional>

namespace tz::wsi::impl
{
	class window_x11
	{
	public:
		window_x11(window_info info);
		window_x11(const window_x11& copy) = delete;
		window_x11(window_x11&& move) = delete;
		~window_x11();
		window_x11& operator=(const window_x11& rhs) = delete;
		window_x11& operator=(window_x11&& rhs) = delete;

		// tz::wsi::window_api<> begin
		using native = Window;
		native get_native() const;
		bool is_close_requested() const;
		tz::vec2ui get_dimensions() const;
		void set_dimensions(tz::vec2ui dimensions);
		std::string get_title() const;
		void set_title(std::string title);
		window_flag::flag_bit get_flags() const;
		void update();
		bool make_opengl_context_current();
		#if TZ_VULKAN
		VkSurfaceKHR make_vulkan_surface(VkInstance vkinst) const;
		#endif // TZ_VULKAN
		const keyboard_state& get_keyboard_state() const;
		const mouse_state& get_mouse_state() const;
		void* get_user_data() const;
		void set_user_data(void* udata);
		// tz::wsi::window_api<> end
	private:
		void impl_request_close();
		void impl_init_opengl();

		Window wnd;
		window_flag::flag_bit flags;
		void* userdata = nullptr;
		keyboard_state kb_state = {};
		mouse_state m_state = {};
		GLXContext ctx = nullptr;
		bool close_requested = false;
	};
	static_assert(tz::wsi::window_api<window_x11>);
	void* get_opengl_proc_address_linux(const char* name);
}

#endif // __linux__
#endif // TZ_WSI_IMPL_LINUX_WINDOW_HPP
