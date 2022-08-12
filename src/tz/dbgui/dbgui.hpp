#ifndef TOPAZ_DBGUI_DBGUI_HPP
#define TOPAZ_DBGUI_DBGUI_HPP
#include "tz/core/game_info.hpp"
#include "tz/gl/device.hpp"
#include "imgui.h"

namespace tz::dbgui
{
	struct InitInfo
	{
		tz::GameInfo game_info;
	};

	void initialise(InitInfo info);
	void terminate();

	void begin_frame();
	void end_frame();
}

#endif // TOPAZ_DBGUI_DBGUI_HPP
