#include "dui/texture_sentinel_tracker.hpp"
#include "gl/texture.hpp"

namespace tz::dui::gl
{
	SentinelTrackerWindow::SentinelTrackerWindow(): DebugWindow("Bindless Texture Sentinel Tracker"){}

	void SentinelTrackerWindow::render()
	{
		ImGui::Begin(this->get_name(), &this->visible);
		ImGui::TextWrapped("tz::gl contains a global TextureSentinel which is responsible for tracking all bindless texture handles (GL_ARB_bindless_texture extension). You should expect to see one of these for each existing terminal texture.");
		if(ImGui::CollapsingHeader("Registered Handles", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for(std::size_t i = 0; i < tz::gl::sentinel().registered_handle_count(); i++)
			{
				// EWWWWWW. This needs to be debug-only code because this is DIS GUS TING
				tz::gl::BindlessTextureHandle cur_handle = tz::gl::sentinel().get_handle(i);
				bool is_resident = tz::gl::sentinel().resident(cur_handle);
				auto tex_name = tz::gl::sentinel().get_texture_name(cur_handle);
				ImGui::Text("Handle %llu (Resident: %s, Name: %lu)", cur_handle, is_resident ? "true" : "false", tex_name.value_or(0));
				if(is_resident && tex_name.has_value())
				{
					GLint cur_bound_tex_name;
					glGetIntegerv(GL_TEXTURE_BINDING_2D, &cur_bound_tex_name);

					GLuint tex_handle = tex_name.value();
					tz::Vector<int, 2> tex_size;
					glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &tex_size[0]);
					glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &tex_size[1]);
					glBindTexture(GL_TEXTURE_2D, tex_handle);
					tz::gl::dui_draw_bindless(cur_handle, tz::Vec2{static_cast<float>(tex_size[0]), static_cast<float>(tex_size[1])});
				};
			}
		}
		ImGui::End();
	}
}