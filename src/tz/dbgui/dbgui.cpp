#include "tz/dbgui/dbgui.hpp"
#include "tz/core/assert.hpp"
#include "tz/core/window.hpp"
#include "tz/gl/renderer.hpp"
#include "imgui.h"

namespace tz::dbgui
{
	bool imgui_impl_tz_init();
	void imgui_impl_tz_term();

	void initialise()
	{
		#if TZ_DEBUG
			bool ret = imgui_impl_tz_init();
			tz_assert(ret, "Failed to initialise tz imgui backend.");
		#endif // TZ_DEBUG
	}

	void terminate()
	{
		#if TZ_DEBUG

		#endif // TZ_DEBUG
	}

	struct TopazPlatformData
	{

	};

	struct TopazRenderData
	{

	};

	TopazPlatformData* global_platform_data = nullptr;
	TopazRenderData* global_render_data = nullptr;

	bool imgui_impl_tz_init()
	{
		ImGui::CreateContext();
		global_platform_data = new TopazPlatformData;
		global_render_data = new TopazRenderData;

		ImGuiIO& io = ImGui::GetIO();
		tz_assert(io.BackendPlatformUserData == NULL, "Already initialised imgui backend!");
		io.BackendPlatformName = "Topaz";
		io.BackendPlatformUserData = global_platform_data;
		io.BackendRendererName = "tz::gl";
		io.BackendRendererUserData = global_render_data;
		return true;
	}

	void imgui_impl_tz_term()
	{
		delete global_platform_data;
		delete global_render_data;
		global_platform_data = nullptr;
		global_render_data = nullptr;

		ImGui::DestroyContext();
	}
}
