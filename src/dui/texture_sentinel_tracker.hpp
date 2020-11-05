#ifndef TOPAZ_GL_IMGUI_TEXTURE_SENTINEL_TRACKER_HPP
#define TOPAZ_GL_IMGUI_TEXTURE_SENTINEL_TRACKER_HPP
#include "dui/imgui_context.hpp"

namespace tz::ext::imgui::gl
{
	class SentinelTrackerWindow : public ImGuiWindow
	{
	public:
		SentinelTrackerWindow();
		virtual void render();
	};
}

#endif // TOPAZ_GL_IMGUI_TEXTURE_SENTINEL_TRACKER_HPP