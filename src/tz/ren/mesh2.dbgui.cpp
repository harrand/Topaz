#include "tz/ren/mesh2.hpp"
#include "imgui.h"


namespace tz::ren
{
	namespace impl
	{
		constexpr float slider_height = 160.0f;

		void vertex_wrangler::dbgui(tz::gl::renderer_handle rh)
		{
			const float vtx_count_kib = this->get_vertex_count() * sizeof(mesh_vertex) / 1024.0f;
			const float vtx_capacity_kib = this->get_vertex_capacity(rh) * sizeof(mesh_vertex) / 1024.0f;
			ImGui::Text("%zu/%zu vertices (%.2f/%.2f KiB) - %.2f%% used",
				this->get_vertex_count(), this->get_vertex_capacity(rh), vtx_count_kib, vtx_capacity_kib, 100.0f * vtx_count_kib / vtx_capacity_kib);

			const float idx_count_kib = this->get_index_count() * sizeof(mesh_index) / 1024.0f;
			const float idx_capacity_kib = this->get_index_capacity(rh) * sizeof(mesh_index) / 1024.0f;
			ImGui::Text("%zu/%zu indices (%.2f/%.2f KiB) - %.2f%% used",
				this->get_index_count(), this->get_index_capacity(rh), idx_count_kib, idx_capacity_kib, 100.0f * idx_count_kib / idx_capacity_kib);

			if(this->mesh_locators.size())
			{
				ImGui::TextColored(ImVec4{1.0f, 0.3f, 0.3f, 1.0f}, "Mesh %d", this->dbgui_mesh_cursor);
				ImGui::VSliderInt("##mesh_id", ImVec2{18.0f, slider_height}, &this->dbgui_mesh_cursor, 0, this->mesh_locators.size() - 1);
				const auto& loc = this->mesh_locators[this->dbgui_mesh_cursor];
				ImGui::SameLine();
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
				if(ImGui::BeginChild("##123", ImVec2(0, slider_height), false, ImGuiWindowFlags_ChildWindow))
				{
					ImGui::Text("Vertex Offset: %u", loc.vertex_offset);
					ImGui::Text("Vertex Count: %u", loc.vertex_count);
					ImGui::Text("Index Offset: %u", loc.index_offset);
					ImGui::Text("Index Count: %u", loc.index_count);
					ImGui::Text("Max Index Value: %u", loc.max_index_value);
				}
				ImGui::EndChild();
			}
		}

		void texture_manager::dbgui(tz::gl::renderer_handle rh)
		{
			ImGui::Text("%zu/%zu textures (%.2f%%)", this->get_texture_count(), this->get_texture_capacity(), 100.0f * this->get_texture_count() / this->get_texture_capacity());
		}

		void render_pass::dbgui_mesh()
		{
			this->vtx.dbgui(this->render);
		}

		void render_pass::dbgui_texture()
		{
			this->tex.dbgui(this->render);
		}

		void render_pass::dbgui_objects()
		{
			ImGui::Text("Well met ;)");
		}
	}

	void mesh_renderer2::dbgui()
	{
		if(ImGui::BeginTabBar("#"))
		{
			if(ImGui::BeginTabItem("Mesh Data"))
			{
				render_pass::dbgui_mesh();
				ImGui::EndTabItem();
			}
			if(ImGui::BeginTabItem("Textures"))
			{
				render_pass::dbgui_texture();
				ImGui::EndTabItem();
			}
			if(ImGui::BeginTabItem("Objects"))
			{
				render_pass::dbgui_objects();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
}