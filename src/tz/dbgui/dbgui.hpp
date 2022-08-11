#ifndef TOPAZ_DBGUI_DBGUI_HPP
#define TOPAZ_DBGUI_DBGUI_HPP
#include "tz/gl/device.hpp"
#include "imgui.h"

namespace tz::dbgui
{
	void initialise();
	void terminate();

	void begin_frame();
	void end_frame();
}

#endif // TOPAZ_DBGUI_DBGUI_HPP
