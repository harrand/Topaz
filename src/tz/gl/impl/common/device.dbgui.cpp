#include "tz/gl/impl/common/device.dbgui.hpp"
#include "tz/gl/device.hpp"

namespace tz::gl
{
	void common_device_dbgui(device_type<tz::gl::renderer_info> auto& device)
	{
		const std::size_t renderer_count = device.renderer_count();

		ImGui::TextColored(ImVec4{1.0f, 0.6f, 0.6f, 1.0f}, "tz::gl::get_device()");
		ImGui::Separator();
		ImGui::TextColored(ImVec4{1.0f, 0.6f, 0.6f, 1.0f}, "Summary");
		ImGui::Text("- The device currently stores %zu renderers", renderer_count);
		ImGui::Text("- The image_format of the window is %s", detail::image_format_strings[static_cast<int>(device.get_window_format())]);
		bool vsync_enabled = device.is_vsync_enabled();
		if(ImGui::Checkbox("Vsync", &vsync_enabled))
		{
			device.set_vsync_enabled(vsync_enabled);
		}
		ImGui::Separator();
		ImGui::TextColored(ImVec4{1.0f, 0.6f, 0.6f, 1.0f}, "Renderers");
		static bool display_internal_renderers = false;
		ImGui::Checkbox("Display Internal Renderers", &display_internal_renderers);
		static int id = 0;
		if(renderer_count == 0)
		{
			return;
		}
		ImGui::SliderInt("Renderer ID", &id, 0, renderer_count - 1);
		ImGui::Indent();
		const auto& renderer = device.get_renderer(static_cast<tz::hanval>(id));
		if(renderer.get_options().contains(tz::gl::renderer_option::_internal) && !display_internal_renderers)
		{
			ImGui::Text("Internal Renderer");
			ImGui::Spacing();
			if(id > 0)
			{
				if(ImGui::Button("<<")){while(device.get_renderer(static_cast<tz::hanval>(id)).get_options().contains(tz::gl::renderer_option::_internal) && id > 0){id--;}}
				ImGui::SameLine();
				if(ImGui::Button("Prev")){id--;}
			}
			if(std::cmp_less(id, (renderer_count - 1)))
			{
				ImGui::SameLine();
				if(ImGui::Button("Next")){id++;}
				ImGui::SameLine();
				if(ImGui::Button(">>")){while(device.get_renderer(static_cast<tz::hanval>(id)).get_options().contains(tz::gl::renderer_option::_internal) && std::cmp_less(id, (renderer_count - 1))){id++;}}
			}
		}
		else
		{
			device.get_renderer(static_cast<tz::hanval>(id)).dbgui();
		}
		ImGui::Unindent();
		ImGui::Separator();
		ImGui::TextColored(ImVec4{1.0f, 0.6f, 0.6f, 1.0f}, "Render Graph");
		if(ImGui::BeginTabBar("#rendergraph"))
		{
			const auto& sched = device.render_graph();
			constexpr ImVec4 edge_default{1.0f, 1.0f, 1.0f, 1.0f};
			constexpr ImVec4 edge_dependency{0.6f, 0.0f, 0.0f, 1.0f};
			constexpr ImVec4 edge_renderwait{0.4f, 0.5f, 1.0f, 1.0f};
			constexpr ImVec4 edge_implicit{0.6f, 0.6f, 0.0f, 1.0f};
			if(ImGui::BeginTabItem("Timeline"))
			{
				static bool hide_internal = true;
				static bool show_chronological_ranks = false;
				ImGui::Checkbox("Hide Internal Renderers", &hide_internal);
				ImGui::Checkbox("Show Chronological Ranks", &show_chronological_ranks);
				auto max_rank = sched.max_chronological_rank();
				for(std::size_t r = 0; r <= max_rank; r++)
				{
					for(std::size_t j = 0; j < sched.timeline.size(); j++)
					{
						auto& ren = device.get_renderer(static_cast<tz::hanval>(sched.timeline[j]));
						if(sched.chronological_rank_eid(sched.timeline[j]) == r)
						{
							std::string namedata{ren.debug_get_name()};
							if(show_chronological_ranks)
							{
								namedata += " (" + std::to_string(r) + ")";
							}
							if(ImGui::Button(namedata.c_str()))
							{
								id = sched.timeline[j];
							}
							ImGui::SameLine();
						}
					}
					ImGui::Spacing();
				}
				#if TZ_DEBUG
				if(!hide_internal)
				{			
					for(std::size_t i = 0; i < 2; i++)
					{
						if(device.renderer_count() > i && ImGui::Button(device.get_renderer(static_cast<tz::hanval>(i)).debug_get_name().data()))
						{
							id = i;
							display_internal_renderers = true;
						}
						if(i < 1)
						{
							ImGui::SameLine();
							ImGui::TextColored(edge_implicit, " -> ");
							ImGui::SameLine();
						}
					}
				}
				#endif
				ImGui::Spacing();
				ImGui::Indent();
				ImGui::Text("("); ImGui::SameLine(); ImGui::TextColored(edge_default, "->"); ImGui::SameLine(); ImGui::Text("Default)"); ImGui::SameLine();
				ImGui::Text("("); ImGui::SameLine(); ImGui::TextColored(edge_dependency, "->"); ImGui::SameLine(); ImGui::Text("Dependency)"); ImGui::SameLine();
				ImGui::Text("("); ImGui::SameLine(); ImGui::TextColored(edge_renderwait, "->"); ImGui::SameLine(); ImGui::Text("Render Wait)"); ImGui::SameLine();
				if(!hide_internal)
				{
					ImGui::Text("("); ImGui::SameLine(); ImGui::TextColored(edge_implicit, "->"); ImGui::SameLine(); ImGui::Text("Internal)");
				}
				ImGui::Unindent();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}

	#if TZ_VULKAN
	template void common_device_dbgui<device_vulkan2>(device_vulkan2&);
	#elif TZ_OGL
	template void common_device_dbgui<device_ogl>(device_ogl&);
	#endif
}