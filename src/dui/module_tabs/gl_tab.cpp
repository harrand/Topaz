#include "core/tz.hpp"
#include "dui/module_tabs/gl_tab.hpp"
#include "dui/window.hpp"

// GL Windows
#include "dui/buffer_tracker.hpp"
#include "dui/tzglp_preview.hpp"
#include "dui/ogl_info.hpp"
#include "dui/texture_sentinel_tracker.hpp"


namespace tz::dui::gl
{
	static gl::BufferTracker tracker{nullptr};
	static tz::gl::Object* obj = nullptr;
    static gl::TZGLPPreview tzglp{};
    static gl::OpenGLInfoWindow oglinfo{};
	static gl::SentinelTrackerWindow textracker{};
    static bool track_objects = false;

    void render_object_tracker();

    void draw_tab()
    {
		static bool wireframe = false;
		static bool cull_backfaces = true;

        if(ImGui::BeginMenu("tz::gl"))
        {
            ImGui::MenuItem("gl::Object Tracking", nullptr, &track_objects);
            ImGui::MenuItem("TZGLP Previewer", nullptr, &tzglp.visible);
            ImGui::MenuItem("Texture Sentinel Tracker", nullptr, &textracker.visible);
            ImGui::MenuItem("OpenGL Info", nullptr, &oglinfo.visible);
            ImGui::MenuItem("Wireframe Mode", nullptr, &wireframe);
            ImGui::MenuItem("Cull Back Faces", nullptr, &cull_backfaces);
            tz::get().enable_wireframe_mode(wireframe);
            tz::get().enable_culling(cull_backfaces);
            ImGui::EndMenu();
        }

        if(track_objects)
        {
            render_object_tracker();
        }

        if(tzglp.visible)
        {
            tzglp.render();
        }

        if(oglinfo.visible)
        {
            oglinfo.render();
        }

        if(textracker.visible)
        {
            textracker.render();
        }

        if(tracker.visible)
        {
            tracker.render();
        }
    }

    void track_object(tz::gl::Object* object)
	{
		obj = object;
		tracker.target_object(object);
	}

    void render_object_tracker()
	{
		ImGui::Begin("tz::gl::Object Tracker", &track_objects);
		if(obj == nullptr)
		{
			ImGui::Text("%s", "No tz::gl::Object is currently being tracked.");
		}
		else
		{
			ImGui::Text("Buffers: %d", obj->size());
			if(ImGui::TreeNode("Attached Buffers"))
			{
				for(std::size_t i = 0; i < obj->size(); i++)
				{
					ImGui::SetNextItemOpen(true, ImGuiCond_Once);
					auto ptr = [](std::size_t i){return reinterpret_cast<void*>(static_cast<std::intptr_t>(i));};
					if(ImGui::TreeNode(ptr(i), "Buffer %zu", i))
					{
						const tz::gl::IBuffer* buf = (*obj)[i];
						std::size_t buf_size_bytes = buf->size();
						if(buf_size_bytes > (1024*1024*1024))
						{
							ImGui::Text("Size: %.1fGiB", static_cast<float>(buf_size_bytes) / (1024 * 1024 * 1024));
						}
						else if(buf_size_bytes > (1024*1024))
						{
							ImGui::Text("Size: %.1fMiB", static_cast<float>(buf_size_bytes) / (1024 * 1024));
						}
						else if(buf_size_bytes > (1024))
						{
							ImGui::Text("Size: %.1fKiB", static_cast<float>(buf_size_bytes) / (1024));
						}
						else
						{
							ImGui::Text("Size: %zuB", buf_size_bytes);
						}
						ImGui::Text("Terminal: %s", buf->is_terminal() ? "true" : "false");
						ImGui::Text("Mapped: %s", buf->is_mapped() ? "true" : "false");
						if(ImGui::Button("Track this Buffer"))
						{
							tracker.track_buffer(i);
							tracker.visible = true;
						}
						ImGui::TreePop();
					}
				}
				ImGui::TreePop();
			}
		}
		ImGui::End();
	}
}