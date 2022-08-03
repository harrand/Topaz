#include "tz/dbgui/dbgui.hpp"
#include "tz/core/assert.hpp"
#include "tz/core/window.hpp"
#include "tz/gl/renderer.hpp"
#include "tz/gl/resource.hpp"
#include "imgui.h"

#include <memory>

namespace tz::dbgui
{
	tz::gl::Device* global_device = nullptr;

	bool imgui_impl_tz_init();
	void imgui_impl_tz_term();

	void initialise(Info info)
	{
		#if TZ_DEBUG
			global_device = info.device;
			bool ret = imgui_impl_tz_init();
			tz_assert(ret, "Failed to initialise tz imgui backend.");
		#endif // TZ_DEBUG
	}

	void terminate()
	{
		#if TZ_DEBUG
			imgui_impl_tz_term();
		#endif // TZ_DEBUG
	}

	void begin_frame()
	{
		#if TZ_DEBUG
			ImGuiIO& io = ImGui::GetIO();
			io.DisplaySize = ImVec2
			{
				static_cast<float>(tz::window().get_width()),
				static_cast<float>(tz::window().get_height())
			};
			ImGui::NewFrame();
		#endif // TZ_DEBUG
	}

	void end_frame()
	{
		#if TZ_DEBUG
			ImGui::EndFrame();
		#endif //TZ_DEBUG
	}

	struct TopazPlatformData
	{

	};

	struct TopazRenderData
	{
		std::unique_ptr<tz::gl::Renderer> renderer = nullptr;
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

		// Make renderer.
		// - Font texture ImageResource.
		unsigned char* font_pixels;
		int font_width, font_height;
		std::span<unsigned char> font_data{font_pixels, font_pixels + (font_width * font_height * 1)};
		io.Fonts->GetTexDataAsRGBA32(&font_pixels, &font_width, &font_height);
		tz::gl::ImageResource font_image = tz::gl::ImageResource::from_memory(tz::gl::ImageFormat::RGBA32, {static_cast<unsigned int>(font_width), static_cast<unsigned int>(font_height)}, font_data, tz::gl::ResourceAccess::StaticFixed);

		tz::gl::RendererInfo rinfo;
		
		global_render_data->renderer = std::make_unique<tz::gl::Renderer>(global_device->create_renderer(rinfo));

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
