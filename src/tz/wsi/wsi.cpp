#include "tz/wsi/wsi.hpp"

#include "tz/wsi/impl/windows/wsi_windows.hpp"
#include "tz/wsi/impl/linux/wsi_linux.hpp"

namespace tz::wsi
{
	void initialise()
	{
		#ifdef _WIN32
			tz::wsi::impl::initialise_windows();
		#elif defined(__linux__)
			tz::wsi::impl::initialise_linux();	
		#endif
	}
	
	void terminate()
	{
		#ifdef _WIN32
			tz::wsi::impl::terminate_windows();
		#elif defined(__linux__)
			tz::wsi::impl::terminate_linux();	
		#endif
	}

	void update()
	{
		#ifdef _WIN32
			tz::wsi::impl::update_windows();
		#elif defined(__linux__)
			tz::wsi::impl::update_linux();	
		#endif
	}
}
