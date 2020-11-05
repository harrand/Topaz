#include "dui/texture_sentinel_tracker.hpp"
#include "gl/texture.hpp"

namespace tz::ext::imgui::gl
{
	SentinelTrackerWindow::SentinelTrackerWindow(): ImGuiWindow("Bindless Texture Sentinel Tracker"){}

	void SentinelTrackerWindow::render()
	{
		ImGui::Begin(this->get_name(), &this->visible);
		ImGui::TextWrapped("tz::gl contains a global TextureSentinel which is responsible for tracking all bindless texture handles (GL_ARB_bindless_texture extension). You should expect to see one of these for each existing terminal texture.");
		if(ImGui::CollapsingHeader("Registered Handles", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for(std::size_t i = 0; i < tz::gl::sentinel().registered_handle_count(); i++)
			{
				tz::gl::BindlessTextureHandle cur_handle = tz::gl::sentinel().get_handle(i);
				ImGui::Text("Handle %llu (Resident: %s)", cur_handle, tz::gl::sentinel().resident(cur_handle) ? "true" : "false");
			}
		}
		ImGui::End();
	}
}