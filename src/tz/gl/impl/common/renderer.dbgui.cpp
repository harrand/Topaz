#include "tz/gl/impl/common/renderer.dbgui.hpp"
#include "tz/gl/device.hpp"

namespace tz::gl
{
	void common_renderer_dbgui(renderer_type auto& renderer)
	{
		if(renderer.is_null())
		{
			ImGui::Text("Null renderer");
			return;
		}
		ImGui::PushID(&renderer);
		ImGui::Text("Renderer Name:");
		ImGui::SameLine();
		ImGui::TextColored(ImVec4{1.0f, 0.6f, 0.6f, 1.0f}, "%s", renderer.debug_get_name().data());
		if(renderer.resource_count() > 0)
		{
			if(ImGui::CollapsingHeader("Resources"))
			{
				unsigned int rcount = renderer.resource_count() - 1;
				ImGui::Text("resource Count: %u", renderer.resource_count());
				static int res_id = 0;
				res_id = std::clamp(res_id, 0, static_cast<int>(rcount));
				ImGui::SliderInt("resource ID:", &res_id, 0, rcount);

				// Display information about current resource.
				ImGui::Indent();
				renderer.get_resource(static_cast<tz::hanval>(res_id))->dbgui();
				ImGui::Unindent();
			}
		}
		else
		{
			ImGui::Text("renderer has no resources.");
		}
		if(ImGui::CollapsingHeader("Renderer State"))
		{
			ImGui::TextColored(ImVec4{1.0f, 0.6f, 0.6f, 1.0f}, "Graphics");
			ImGui::Indent();
			// Graphics - Index Buffer
			{
				auto han = renderer.get_state().graphics.index_buffer;
				auto* comp = renderer.get_component(han);
				if(comp != nullptr && ImGui::CollapsingHeader("Index Buffer"))
				{
					ImGui::Text("[resource %zu]", static_cast<std::size_t>(static_cast<tz::hanval>(han)));
					ImGui::Indent();
					comp->get_resource()->dbgui();
					ImGui::Unindent();
				}
			}
			// Graphics - Draw Buffer
			{
				auto han = renderer.get_state().graphics.draw_buffer;
				auto* comp = renderer.get_component(han);
				if(comp != nullptr && ImGui::CollapsingHeader("Draw Indirect Buffer"))
				{
					ImGui::Text("[resource %zu]", static_cast<std::size_t>(static_cast<tz::hanval>(han)));
					ImGui::Indent();
					comp->get_resource()->dbgui();
					ImGui::Unindent();
				}
			}
			ImGui::Spacing();
			// Graphics - Clear Colour
			{
				auto col = renderer.get_state().graphics.clear_colour;
				if(ImGui::DragFloat4("Clear Colour", col.data().data(), 0.02f, 0.0f, 1.0f))
				{
					renderer.edit
					(
						tz::gl::RendererEditBuilder{}
						.render_state
						({
							.clear_colour = col
						})
						.build()
					);
				}
			}
			// Graphics - Tri Count
			{
				if(renderer.get_state().graphics.draw_buffer != tz::nullhand)
				{
					ImGui::Text("Triangle Count: Indirect Buffer");
				}
				else
				{
					ImGui::Text("Triangle Count: %zu", renderer.get_state().graphics.tri_count);
				}
			}
			// Graphics - Wireframe Mode
			{
				bool wfm = renderer.get_state().graphics.wireframe_mode;
				if(ImGui::Checkbox("Wireframe Mode", &wfm))
				{
					renderer.edit
					(
						tz::gl::RendererEditBuilder{}
						.render_state
						({
							.wireframe_mode = wfm
						})
						.build()
					);
				}
			}
			// Graphics - Culling
			{
				const char* culling_strings[] = {"None", "Front", "Back", "Both"};
				static const char* current_culling_string = culling_strings[static_cast<int>(renderer.get_state().graphics.culling)];
				if(ImGui::BeginCombo("Culling Mode", current_culling_string))
				{
					for(int i = 0; i < 4; i++)
					{
						bool is_selected = (current_culling_string) == culling_strings[i];
						if(ImGui::Selectable(culling_strings[i], is_selected))
						{
							current_culling_string = culling_strings[i];
							renderer.edit
							(
								tz::gl::RendererEditBuilder{}
								.render_state
								({
									.culling = static_cast<tz::gl::graphics_culling>(i)
								})
								.build()
							);
						}
						if(is_selected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}
			}

			ImGui::Unindent();
			ImGui::TextColored(ImVec4{1.0f, 0.6f, 0.6f, 1.0f}, "Compute");
			ImGui::Indent();
			// Compute - Kernel
			{
				auto kern = static_cast<tz::vec3i>(renderer.get_state().compute.kernel);
				if(ImGui::DragInt3("Kernel", kern.data().data(), 0.25f, 0, 64))
				{
					renderer.edit
					(
						tz::gl::RendererEditBuilder{}
						.compute
						({
							.kernel = kern
						})
						.build()
					);
				}
			}
			ImGui::Unindent();
		}
		if(!renderer.get_options().empty() && ImGui::CollapsingHeader("Renderer Options"))
		{
			for(renderer_option option : renderer.get_options())
			{
				ImGui::Text("%s", detail::renderer_option_strings[static_cast<int>(option)]);
			}
			#if TZ_DEBUG
				ImGui::PushTextWrapPos();
				ImGui::TextDisabled("Note: In debug builds, extra options might be present that you did not ask for. These are added to allow the debug-ui to display ontop of your rendered output.");
				ImGui::PopTextWrapPos();
			#endif //TZ_DEBUG
		}
		ImGui::Separator();
		ImGui::PopID();
	}

	#if TZ_VULKAN
	template void common_renderer_dbgui<renderer_vulkan2>(renderer_vulkan2&);
	#elif TZ_OGL
	template void common_renderer_dbgui<renderer_ogl>(renderer_ogl&);
	#endif
}