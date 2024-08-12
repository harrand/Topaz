#include "tz/ren/mesh.hpp"
#include "imgui.h"
#include <sstream>


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

			ImGui::Text("%zu meshes", this->get_mesh_count());

			if(this->mesh_locators.size())
			{
				ImGui::TextColored(ImVec4{1.0f, 0.3f, 0.3f, 1.0f}, "Mesh %d", this->dbgui_mesh_cursor);
				if(ImGui::Button("+") && this->dbgui_mesh_cursor < static_cast<int>(this->mesh_locators.size() - 1))
				{
					this->dbgui_mesh_cursor++;
				}
				ImGui::VSliderInt("##mesh_id", ImVec2{18.0f, slider_height}, &this->dbgui_mesh_cursor, 0, this->mesh_locators.size() - 1);
				const auto& loc = this->mesh_locators[static_cast<tz::hanval>(this->dbgui_mesh_cursor)];
				ImGui::SameLine();
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
				if(ImGui::BeginChild("##123", ImVec2(0, slider_height), false, ImGuiWindowFlags_ChildWindow))
				{
					if(loc == mesh_locator{})
					{
						ImGui::Text("Mesh %d is in free-list", this->dbgui_mesh_cursor);;
					}
					else
					{
						ImGui::Text("Vertex Offset: %u", loc.vertex_offset);
						ImGui::Text("Vertex Count: %u", loc.vertex_count);
						ImGui::Text("Index Offset: %u", loc.index_offset);
						ImGui::Text("Index Count: %u", loc.index_count);
					}
				}
				ImGui::EndChild();
				if(ImGui::Button("-") && this->dbgui_mesh_cursor > 0)
				{
					this->dbgui_mesh_cursor--;
				}
			}
		}

		void texture_manager::dbgui(tz::gl::renderer_handle rh)
		{
			(void)rh;
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
				this->tree.dbgui(true);
			}

			if(this->get_object_count(true))
			{
				ImGui::TextColored(ImVec4{1.0f, 0.3f, 0.3f, 1.0f}, "Object %d", this->dbgui_object_cursor);
				if(ImGui::Button("+") && this->dbgui_object_cursor < static_cast<int>(this->get_object_count(true) - 1))
				{
					this->dbgui_object_cursor++;
				}
				ImGui::VSliderInt("##object_id", ImVec2{18.0f, slider_height}, &this->dbgui_object_cursor, 0, this->get_object_count(true) - 1);
				object_handle oh = static_cast<tz::hanval>(this->dbgui_object_cursor);
				auto& obj = this->obj.get_object_internals(this->render)[this->dbgui_object_cursor];
				ImGui::SameLine();
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
				if(ImGui::BeginChild("##1234", ImVec2(0, slider_height), false, ImGuiWindowFlags_ChildWindow))
				{
					if(this->object_is_in_free_list(oh))
					{
						ImGui::Text("Object %d is in free-list", this->dbgui_object_cursor);
					}
					else
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

						std::string parent_str;
						object_handle parent = this->object_get_parent(oh);
						if(parent == tz::nullhand)
						{
							parent_str = "none";
						}
						else
						{
							parent_str = std::to_string(static_cast<unsigned int>(static_cast<tz::hanval>(parent)));
						}
						ImGui::Text("Parent");
						ImGui::SameLine();
						if(ImGui::InputText("##parent", &parent_str, ImGuiInputTextFlags_EnterReturnsTrue))
						{
							std::stringstream sstr;
							sstr.str(parent_str);
							unsigned int new_parent;
							sstr >> new_parent;
							if(new_parent < this->get_object_count())
							{
								this->object_set_parent(oh, static_cast<tz::hanval>(new_parent));
							}
						}
						if(parent != tz::nullhand)
						{
							ImGui::SameLine();
							if(ImGui::Button("Unparent"))
							{
								this->object_set_parent(oh, tz::nullhand);
							}
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
				}
				ImGui::EndChild();
				if(ImGui::Button("-") && this->dbgui_object_cursor > 0)
				{
					this->dbgui_object_cursor--;
				}
			}
		}
	}

	void mesh_renderer::dbgui(bool include_operations)
	{
		if(ImGui::BeginTabItem("Camera"))
		{
			this->camera_transform.dbgui();
			ImGui::EndTabItem();
		}
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
		if(include_operations && ImGui::BeginTabItem("Debug Operations"))
		{
			this->dbgui_operations();
			ImGui::EndTabItem();
		}
	}

	void mesh_renderer::dbgui_operations()
	{
		if(ImGui::TreeNode("Mesh Operations"))
		{
			for(std::size_t mesh_id = 0; mesh_id < this->get_mesh_count(true); mesh_id++)
			{
				auto hanval = static_cast<tz::hanval>(mesh_id);
				const mesh_locator& mloc = this->get_mesh(hanval);
				ImGui::Text("Mesh %zu", mesh_id);
				ImGui::SameLine();
				std::string button_label = "Remove##" + std::to_string(mesh_id);
				if(mloc != mesh_locator{} && ImGui::Button(button_label.c_str()))
				{
					this->remove_mesh(hanval);
				}
				else if(mloc == mesh_locator{})
				{
					ImGui::Text("(Free-list)");
				}
			}
			ImGui::TreePop();
		}

		if(ImGui::TreeNode("Object Operations"))
		{
			for(std::size_t i = 0; i < this->get_object_count(true); i++)
			{
				auto hanval = static_cast<tz::hanval>(i);
				ImGui::Text("Object %zu", i);
				ImGui::SameLine();
				std::string button_label = "Remove##" + std::to_string(i);
				if(!this->object_is_in_free_list(hanval) && ImGui::Button(button_label.c_str()))
				{
					this->remove_object(hanval);
				}
				else if(this->object_is_in_free_list(hanval))
				{
					ImGui::Text("(Free-list)");
				}
			}
			ImGui::TreePop();
		}
	}
}
