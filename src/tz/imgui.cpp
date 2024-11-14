#include "tz/imgui.hpp"
#include "tz/topaz.hpp"
#include "tz/gpu/resource.hpp"
#include "tz/gpu/hardware.hpp"
#include <span>

namespace tz::detail
{
	// imgui backend initialise.
	struct render_data
	{
		tz::gpu::resource_handle font_image = tz::nullhand;
	} render;

	void imgui_initialise()
	{
		ImGui::CreateContext();
		if(!gpu::hardware_in_use())
		{
			gpu::use_hardware(gpu::find_best_hardware());
		}

		ImGuiIO& io = ImGui::GetIO();
		io.BackendPlatformName = "Topaz";
		io.BackendRendererName = "Topaz";

		// make font texture resource.
		unsigned char* fontptr;
		int fontw, fonth;
		io.Fonts->GetTexDataAsRGBA32(&fontptr, &fontw, &fonth);
		std::span<unsigned char> fontdata{fontptr, static_cast<std::size_t>(fontw * fonth * 4u)};

		render.font_image = tz_must(tz::gpu::create_image
		({
			.width = static_cast<unsigned int>(fontw),
			.height = static_cast<unsigned int>(fonth),
			.data = std::as_bytes(fontdata),
			.name = "ImGui Font image"
		}));
	}

	void imgui_terminate()
	{

		ImGui::DestroyContext();
	}
}