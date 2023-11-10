#include "tz/ren/animation.hpp"

namespace tz::ren
{
	constexpr float slider_height = 160.0f;

//--------------------------------------------------------------------------------------------------

	void animation_renderer::dbgui()
	{
		mesh_renderer::dbgui(false);
		if(ImGui::BeginTabItem("Animations"))
		{
			this->dbgui_animations();
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("Operations"))
		{
			mesh_renderer::dbgui_operations();
			this->dbgui_animation_operations();
			ImGui::EndTabItem();
		}
	}

//--------------------------------------------------------------------------------------------------

	void animation_renderer::dbgui_animations()
	{
		const float joint_count_kib = this->get_joint_count() * sizeof(std::uint32_t) / 1024.0f;
		const float joint_capacity_kib = this->get_joint_capacity() * sizeof(std::uint32_t) / 1024.0f;
			ImGui::Text("%zu/%zu joints (%.2f/%.2f KiB) - %.2f%% used",
				this->get_joint_count(), this->get_joint_capacity(), joint_count_kib, joint_capacity_kib, 100.0f * joint_count_kib / joint_capacity_kib);

		if(this->animated_objects.size())
		{
			ImGui::TextColored(ImVec4{1.0f, 0.3f, 0.3f, 1.0f}, "Animated Object %d", this->dbgui_animated_objects_cursor);
			if(ImGui::Button("+") && this->dbgui_animated_objects_cursor < static_cast<int>(this->animated_objects.size() - 1))
			{
				this->dbgui_animated_objects_cursor++;
			}
			ImGui::VSliderInt("##anim_id", ImVec2{18.0f, slider_height}, &this->dbgui_animated_objects_cursor, 0, this->animated_objects.size() - 1);	
			auto& anim_objects = this->animated_objects[this->dbgui_animated_objects_cursor];
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
					if(anim_objects.objects.size() && ImGui::CollapsingHeader("Transform"))
					{
						mesh_renderer::get_hierarchy().dbgui_node(static_cast<std::size_t>(static_cast<tz::hanval>(anim_objects.objects.front())), true);	
					}
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

					ImGui::Spacing();

					if(gltf.data.get_animations().size())
					{
						// choose an animation to play/queue
						ImGui::TextColored(ImVec4{1.0f, 0.3f, 0.3f, 1.0f}, "Play/Queue Animation");
						static playback_data new_anim;	
						ImGui::Checkbox("Loop", &new_anim.loop);
						ImGui::SliderFloat("Time Warp", &new_anim.time_warp, -5.0f, 5.0f);
						ImGui::SliderInt("Animation", reinterpret_cast<int*>(&new_anim.animation_id), 0, gltf.data.get_animations().size() - 1, "%zu");
						ImGui::Text("%s", this->gltf_get_animation_name(static_cast<tz::hanval>(gltf_id), new_anim.animation_id).data());
						if(ImGui::Button("Play"))
						{
							this->animated_object_play_animation(aoh, new_anim);
						}
						if(ImGui::Button("Queue"))
						{
							this->animated_object_queue_animation(aoh, new_anim);
						}
					}

					if(anim_objects.playback.size() && ImGui::CollapsingHeader("Playing Animations", ImGuiTreeNodeFlags_DefaultOpen))
					{
						ImGui::Separator();
						auto& currently_playing = anim_objects.playback.front();
						std::string_view animation_name = this->gltf_get_animation_name(static_cast<tz::hanval>(gltf_id), currently_playing.animation_id);
						ImGui::Text("%zu (%s)", currently_playing.animation_id, animation_name.data());
						ImGui::ProgressBar(anim_objects.playback_time / this->gltf_get_animation_length(static_cast<tz::hanval>(gltf_id), currently_playing.animation_id));
						ImGui::Checkbox("Loop##1", &currently_playing.loop);
						ImGui::SliderFloat("Time Warp##1", &currently_playing.time_warp, -5.0f, 5.0f);
						if(ImGui::Button("Skip"))
						{
							this->animated_object_skip_animation(aoh);
						}
					}
				}
			}
			ImGui::EndChild();

			if(ImGui::Button("-") && this->dbgui_animated_objects_cursor > 0)
			{
				this->dbgui_animated_objects_cursor--;
			}
		}
	}

//--------------------------------------------------------------------------------------------------

	void animation_renderer::dbgui_animation_operations()
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
						(void)handle;
					#endif
					
				}
				ImGui::TreePop();
			}
			for(std::size_t i = 0; i < this->gltfs.size(); i++)
			{
				gltf_handle hanval = static_cast<tz::hanval>(i);
				ImGui::Text("GLTF %zu", i);
				ImGui::SameLine();
				std::string button_label = "Remove##" + std::to_string(i);
				if(!this->gltf_is_in_free_list(hanval) && ImGui::Button(button_label.c_str()))
				{
					this->remove_gltf(hanval);
				}
				else if(this->gltf_is_in_free_list(hanval))
				{
					ImGui::Text("(Free-list)");
				}

			}
			ImGui::TreePop();
		}
		if(ImGui::TreeNode("Animated Objects Operations"))
		{
			if(this->gltfs.size() && ImGui::TreeNode("Clone from GLTF"))
			{
				static int gltf_id = 0;
				ImGui::SliderInt("GLTF id", &gltf_id, 0, this->gltfs.size() - 1);
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
			for(std::size_t i = 0; i < this->animated_objects.size(); i++)
			{
				auto hanval = static_cast<tz::hanval>(i);
				ImGui::Text("Animated Objects %zu", i);
				ImGui::SameLine();
				std::string button_label = "Remove##" + std::to_string(i);
				if(!this->animated_objects_are_in_free_list(hanval) && ImGui::Button(button_label.c_str()))
				{
					this->remove_animated_objects(hanval);
				}
				else if(this->animated_objects_are_in_free_list(hanval))
				{
					ImGui::Text("(Free-list)");
				}
			}
			ImGui::TreePop();
		}
	}
}