#ifndef TZ_WSI_IMPL_WINDOWS_WINDOW_HPP
#define TZ_WSI_IMPL_WINDOWS_WINDOW_HPP
#ifdef _WIN32
#include "tz/wsi/api/window.hpp"
#include "tz/wsi/impl/windows/detail/winapi.hpp"

namespace tz::wsi::impl
{
	class window_winapi
	{
	public:
		window_winapi(window_info info);
		window_winapi(const window_winapi& copy) = delete;
		window_winapi(window_winapi&& move) = delete;
		~window_winapi();
		window_winapi& operator=(const window_winapi& rhs) = delete;
		window_winapi& operator=(window_winapi&& rhs) = delete;

		// tz::wsi::window_api<> begin
		using native = HWND;
		native get_native() const;
		bool is_close_requested() const;
		hdk::vec2ui get_dimensions() const;
		void set_dimensions(hdk::vec2ui dimensions);
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
		friend LRESULT wndproc(HWND, UINT, WPARAM, LPARAM);
	private:
		void impl_init_opengl();
		bool impl_is_opengl() const;
		void impl_request_close();
		keyboard_state& impl_mutable_keyboard_state();
		mouse_state& impl_mutable_mouse_state();

		HWND hwnd = nullptr;
		HDC hdc = nullptr;
		HGLRC opengl_rc = nullptr;
		window_flag::flag_bit flags = {};
		bool close_requested = false;
		keyboard_state key_state = {};
		mouse_state mouse_state = {};
		void* userdata = nullptr;
	};
	static_assert(tz::wsi::window_api<window_winapi>);

	void* get_opengl_proc_address_windows(const char* name);
}

#endif // _WIN32
#endif // TZ_WSI_IMPL_WINDOWS_WINDOW_HPP
