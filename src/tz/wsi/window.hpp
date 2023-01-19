#ifndef TZ_WSI_WINDOW_HPP
#define TZ_WSI_WINDOW_HPP
#include "tz/wsi/impl/windows/window.hpp"
#include "tz/wsi/impl/linux/window.hpp"
#include "tz/core/data/handle.hpp"

namespace tz::wsi
{
	namespace detail
	{
		struct window_handle_tag{};
	}
	/**
	 * @ingroup tz_wsi_window
	 * An opaque handle associated with a window.
	 */
	using window_handle = tz::handle<detail::window_handle_tag>;

	#define DOCONLY TZ_VULKAN && TZ_OGL
	#if DOCONLY
		/**
		 * @ingroup tz_wsi_window
		 * @struct tz::wsi::window
		 * @brief Represents an application window. Implements @ref tz::wsi::window_api.
		 */

		/**
		 * @ingroup tz_wsi_window
		 * @fn void* tz::wsi::get_opengl_proc_address(const char* name)
		 * Retrieve the address of an OpenGL core function, if it is supported by the current context.
		 * @pre An OpenGL context must be current, usually from a window.
		 * @return Pointer to OpenGL function. If failure occurs, either by the function not being supported by the current context, or no current context at all, then `nullptr` is returned.
		 */

		/**
		 * @ingroup tz_wsi_window
		 * @fn void wait_for_event()
		 * Block the current thread until any Topaz window receives a window event.
		 */
		struct window{};
	#else
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
				tz::error("wait_for_event on linux is NYI");
			}
		#else
			static_assert(false, "Window Implementation does not seem to exist for this platform.");
		#endif
	#endif

	/**
	 * @ingroup tz_wsi_window
	 * Create a new window.
	 * @param info Creation flags for the new window.
	 * @return Opaque handle associated with the new window. Keep ahold of it.
	 */
	window_handle create_window(window_info info = {});
	/**
	 * @ingroup tz_wsi_window
	 * Destroy an existing window associated with the provided handle.
	 * @param wh Opaque window handle, retrieved from a prior call to @ref tz::wsi::create_window
	 * @return True if the window was successfully destroyed, false otherwise.
	 */
	bool destroy_window(window_handle wh);
	/**
	* @ingroup tz_wsi_window
	* Retrieve a window associated with the provided handle.
	* @param wh Window handle retrieved from a prior invocation of @ref tz::wsi::create_window
	* @return Reference to the associated window.
	*/
	window& get_window(window_handle wh);
	/**
 	 * @ingroup tz_wsi_window
	 * Query as to whether there exists a window associated with the provided handle.
	 * @return True if there is a valid window associated with the handle that has not been deleted, otherwise false.
	 */
	bool has_window(window_handle wh);
	/**
	* @ingroup tz_wsi_window
	* Retrieve a count of all valid windows that have not yet been deleted.
	*/
	std::size_t window_count();
}

#endif // TZ_WSI_WINDOW_HPP
