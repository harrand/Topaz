#ifndef TZ_WSI_WINDOW_HPP
#define TZ_WSI_WINDOW_HPP
#include "tz/wsi/impl/windows/window.hpp"
#include "tz/wsi/impl/linux/window.hpp"
#include "hdk/data/handle.hpp"

namespace tz::wsi
{
	namespace detail
	{
		struct window_handle_tag{};
	}
	using window_handle = hdk::handle<detail::window_handle_tag>;

	#ifdef _WIN32
		using window = impl::window_winapi;
		#if TZ_VULKAN
			#define TZ_WSI_VULKAN_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
		#endif
		inline void* get_opengl_proc_address(const char* name)
		{
			return impl::get_opengl_proc_address_windows(name);
		}
		inline void wait_for_event()
		{
			WaitMessage();
		}
	#elif defined(__linux__)
		using window = impl::window_x11;
		#if TZ_VULKAN
			#define TZ_WSI_VULKAN_EXTENSION_NAME VK_KHR_XLIB_SURFACE_EXTENSION_NAME;
		#endif
		inline void* get_opengl_proc_address(const char* name)
		{
			return impl::get_opengl_proc_address_linux(name);
		}
		inline void wait_for_event()
		{
			hdk::error("wait_for_event on linux is NYI");
		}
	#else
		static_assert(false, "Window Implementation does not seem to exist for this platform.");
	#endif

	window_handle create_window(window_info info = {});
	bool destroy_window(window_handle wh);
	window& get_window(window_handle wh);
	bool has_window(window_handle wh);
	std::size_t window_count();
}

#endif // TZ_WSI_WINDOW_HPP