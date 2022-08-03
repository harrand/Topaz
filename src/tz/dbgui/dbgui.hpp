#ifndef TOPAZ_DBGUI_DBGUI_HPP
#define TOPAZ_DBGUI_DBGUI_HPP
#include "tz/gl/device.hpp"

namespace tz::dbgui
{
	struct Info
	{
		tz::gl::Device* device;
	};

	void initialise(Info info);
	void terminate();

	void begin_frame();
	void end_frame();
}

#endif // TOPAZ_DBGUI_DBGUI_HPP
