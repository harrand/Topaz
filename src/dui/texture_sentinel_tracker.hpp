#ifndef TOPAZ_GL_IMGUI_TEXTURE_SENTINEL_TRACKER_HPP
#define TOPAZ_GL_IMGUI_TEXTURE_SENTINEL_TRACKER_HPP
#include "dui/window.hpp"

namespace tz::dui::gl
{
	class SentinelTrackerWindow : public DebugWindow
	{
	public:
		SentinelTrackerWindow();
		virtual void render();
	};
}

#endif // TOPAZ_GL_IMGUI_TEXTURE_SENTINEL_TRACKER_HPP