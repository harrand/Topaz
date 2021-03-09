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
	static bool culling = false;

	void render_face_culling();
    void render_object_tracker();

    void draw_tab()
    {
		static bool wireframe = false;

        if(ImGui::BeginMenu("tz::gl"))
        {
            ImGui::MenuItem("gl::Buffer Tracking", nullptr, &track_objects);
            ImGui::MenuItem("TZGLP Previewer", nullptr, &tzglp.visible);
            ImGui::MenuItem("Texture Sentinel Tracker", nullptr, &textracker.visible);
            ImGui::MenuItem("OpenGL Info", nullptr, &oglinfo.visible);
            ImGui::MenuItem("Wireframe Mode", nullptr, &wireframe);
            ImGui::MenuItem("Face Culling", nullptr, &culling);
            tz::get().render_settings().enable_wireframe_mode(wireframe);
            ImGui::EndMenu();
        }

		if(culling)
		{
			render_face_culling();
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

	void render_face_culling()
	{
		ImGui::Begin("Face Culling", &culling);
		// Get current culling mode
		tz::RenderSettings::CullTarget current_target = tz::get().render_settings().get_culling();
		auto tarv = reinterpret_cast<int*>(&current_target);
		bool changed = false;
		changed |= ImGui::RadioButton("Back Faces", tarv, static_cast<int>(tz::RenderSettings::CullTarget::BackFaces));
		changed |= ImGui::RadioButton("Front Faces", tarv, static_cast<int>(tz::RenderSettings::CullTarget::FrontFaces));
		changed |= ImGui::RadioButton("Both Faces", tarv, static_cast<int>(tz::RenderSettings::CullTarget::Both));
		changed |= ImGui::RadioButton("No Culling", tarv, static_cast<int>(tz::RenderSettings::CullTarget::Nothing));

		if(changed)
		{
			tz::get().render_settings().set_culling(current_target);
		}

		ImGui::End();
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