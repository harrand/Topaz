#include "tz/ren/animation2.hpp"

namespace tz::ren
{
	constexpr float slider_height = 160.0f;

//--------------------------------------------------------------------------------------------------

	void animation_renderer2::dbgui()
	{
		mesh_renderer2::dbgui(false);
		if(ImGui::BeginTabItem("Animations"))
		{
			this->dbgui_animations();
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("Operations"))
		{
			mesh_renderer2::dbgui_operations();
			this->dbgui_animation_operations();
			ImGui::EndTabItem();
		}
	}

//--------------------------------------------------------------------------------------------------

	void animation_renderer2::dbgui_animations()
	{
		const float joint_count_kib = this->get_joint_count() * sizeof(std::uint32_t) / 1024.0f;
		const float joint_capacity_kib = this->get_joint_capacity() * sizeof(std::uint32_t) / 1024.0f;
			ImGui::Text("%zu/%zu joints (%.2f/%.2f KiB) - %.2f%% used",
				this->get_joint_count(), this->get_joint_capacity(), joint_count_kib, joint_capacity_kib, 100.0f * joint_count_kib / joint_capacity_kib);

		if(this->animated_objects.size())
		{
			ImGui::TextColored(ImVec4{1.0f, 0.3f, 0.3f, 1.0f}, "Animated Object %d", this->dbgui_animated_objects_cursor);
			ImGui::VSliderInt("##anim_id", ImVec2{18.0f, slider_height}, &this->dbgui_animated_objects_cursor, 0, this->animated_objects.size() - 1);	
			const auto& anim_objects = this->animated_objects[this->dbgui_animated_objects_cursor];
			animated_objects_handle aoh = static_cast<tz::hanval>(this->dbgui_animated_objects_cursor);
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
			if(ImGui::BeginChild("##12345", ImVec2(0, slider_height), false, ImGuiWindowFlags_ChildWindow))
			{
				if(this->animated_objects_are_in_free_list(aoh))
				{
					ImGui::Text("Animated Object %d is in free-list", this->dbgui_animated_objects_cursor);
				}
				else
				{
					std::size_t gltf_id = static_cast<std::size_t>(static_cast<tz::hanval>(anim_objects.gltf));
					ImGui::Text("Associated with: GLTF %zu", gltf_id);
					const auto& gltf = this->gltfs[gltf_id];
					if(anim_objects.joint_count > 0)
					{
						ImGui::Text("Joint Region: %zu-%zu (%zu joints)", anim_objects.joint_buffer_offset, (anim_objects.joint_buffer_offset + anim_objects.joint_count - 1), anim_objects.joint_count);
					}
					else
					{
						ImGui::Text("Not an animated object.");
					}
					std::string objects_label = "Objects (" + std::to_string(anim_objects.objects.size()) + ")";
					if(ImGui::TreeNode(objects_label.c_str()))
					{
						for(object_handle oh : anim_objects.objects)
						{
							ImGui::Text("Object %zu", static_cast<std::size_t>(static_cast<tz::hanval>(oh)));
						}
						ImGui::TreePop();
					}	
				}
			}
			ImGui::EndChild();
		}
	}

//--------------------------------------------------------------------------------------------------

	void animation_renderer2::dbgui_animation_operations()
	{
		if(ImGui::TreeNode("GLTF Operations"))
		{
			if(ImGui::TreeNode("Load from File"))
			{
				static std::string load_path;
				ImGui::InputText("Path", &load_path);
				if(ImGui::Button("Add"))
				{
					auto gltf = tz::io::gltf::from_file(load_path.c_str());
					gltf_handle handle = this->add_gltf(gltf);
					#if TZ_DEBUG
						tz::report("GLTF handle %zu loaded from path \"%s\"", static_cast<std::size_t>(static_cast<tz::hanval>(handle)), load_path.c_str());
					#else
						void(handle);
					#endif
					
				}
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}
		if(ImGui::TreeNode("Animated Objects Operations"))
		{
			if(this->gltfs.size() && ImGui::TreeNode("Clone from GLTF"))
			{
				static int gltf_id = 0;
				ImGui::SliderInt("GLTF id", &gltf_id, 0, this->gltfs.size() - 1);
				const auto& gltf = this->gltfs[gltf_id];
				if(ImGui::Button("Clone"))
				{
					auto handle = this->add_animated_objects
					({
						.gltf = static_cast<tz::hanval>(gltf_id),
					});
					#if TZ_DEBUG
						tz::report("Animated Objects handle %zu loaded from GLTF %d", static_cast<std::size_t>(static_cast<tz::hanval>(handle)), gltf_id);
					#else
						(void)handle;
					#endif
				}
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}
	}
}