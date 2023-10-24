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

			ImGui::Text("%zu meshes (%zu free-list)", this->mesh_locators.size(), this->mesh_handle_free_list.size());

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
			const float obj_count_kib = this->compute.get_draw_count() * sizeof(object_data) / 1024.0f;
			const float obj_capacity_kib = this->compute.get_draw_capacity() * sizeof(object_data) / 1024.0f;
			ImGui::Text("%zu/%zu objects (%zu free-list) (%.2f/%.2f KiB) - %.2f%% used",
				this->compute.get_draw_count(), this->compute.get_draw_capacity(), this->compute.get_draw_free_list_count(), obj_count_kib, obj_capacity_kib, 100.0f * obj_count_kib / obj_capacity_kib);

			if(ImGui::CollapsingHeader("Tree View"))
			{
				this->tree.dbgui(false);
			}

			if(this->compute.get_draw_count())
			{
				ImGui::TextColored(ImVec4{1.0f, 0.3f, 0.3f, 1.0f}, "Object %d", this->dbgui_object_cursor);
				ImGui::VSliderInt("##object_id", ImVec2{18.0f, slider_height}, &this->dbgui_object_cursor, 0, this->compute.get_draw_count() - 1);
				auto& obj = this->obj.get_object_internals(this->render)[this->dbgui_object_cursor];
				ImGui::SameLine();
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
				if(ImGui::BeginChild("##1234", ImVec2(0, slider_height), false, ImGuiWindowFlags_ChildWindow))
				{
					//// Object View ////
					// Colour Tint
					ImGui::SliderFloat3("Colour Tint", obj.colour_tint.data().data(), 0.0f, 1.0f);

					// Visibility
					bool visible = this->compute.get_visibility_at(this->dbgui_object_cursor);
					if(ImGui::Checkbox("Visibility", &visible))
					{
						this->compute.set_visibility_at(this->dbgui_object_cursor, visible);
					}

					mesh_locator loc = this->compute.get_mesh_at(this->dbgui_object_cursor);
					mesh_handle mesh = this->vtx.try_find_mesh_handle(loc);
					if(mesh == tz::nullhand)
					{
						ImGui::Text("<No Mesh>");
					}
					else
					{
						ImGui::Text("Mesh: %zu", static_cast<std::size_t>(static_cast<tz::hanval>(mesh)));
						if(ImGui::TreeNode("View Mesh Info"))
						{
							ImGui::Text("Vertex Offset: %u", loc.vertex_offset);
							ImGui::Text("Vertex Count: %u", loc.vertex_count);
							ImGui::Text("Index Offset: %u", loc.index_offset);
							ImGui::Text("Index Count: %u", loc.index_count);
							ImGui::Text("Max Index Value: %u", loc.max_index_value);
							ImGui::TreePop();
						}
					}
					if(ImGui::TreeNode("View Texture Info"))
					{
						for(std::size_t i = 0; i < obj.bound_textures.size() && !obj.bound_textures[i].is_null(); i++)
						{
							auto& texloc = obj.bound_textures[i];
							ImGui::Text("Binding %zu = Texture %zu", i, static_cast<std::size_t>(static_cast<tz::hanval>(texloc.texture)));
							ImGui::SliderFloat3("Colour Tint", texloc.colour_tint.data().data(), 0.0f, 1.0f);
							ImGui::Spacing();
						}
						ImGui::TreePop();
					}
				}
				ImGui::EndChild();
			}
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