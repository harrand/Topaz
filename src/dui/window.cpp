#include "dui/window.hpp"
#include "gl/object.hpp"
#include "dui/imgui_impl_opengl3.h"
#include "dui/imgui_impl_glfw.h"
#include "glfw/glfw3.h"
#include "core/debug/assert.hpp"
#include "core/tz.hpp"

// Diagnostics
#include <vector>
#include "dui/buffer_tracker.hpp"
#include "dui/tzglp_preview.hpp"
#include "dui/ogl_info.hpp"
#include "dui/texture_sentinel_tracker.hpp"

namespace tz::dui
{
	static std::vector<std::unique_ptr<DebugWindow>> windows;
	static tz::gl::Object* obj = nullptr;
	static gl::BufferTracker tracker{nullptr};
	static gl::TZGLPPreview tzglp{};
	static gl::OpenGLInfoWindow oglinfo{};
	static gl::SentinelTrackerWindow textracker{};
	static bool show_demo_window = false;

	DebugWindow::DebugWindow(const char* name): name(name){}

	const char* DebugWindow::get_name() const
	{
		return this->name;
	}

	void set_window_impl(GLFWwindow* wnd)
	{
		window = wnd;
	}

	void track_object(tz::gl::Object* object)
	{
		obj = object;
		tracker.target_object(object);
	}

	void initialise()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 430");
	}

	void terminate()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	DebugWindow& add_window(std::unique_ptr<DebugWindow> wnd)
	{
		wnd->visible = false;
		windows.push_back(std::move(wnd));
		return *windows.back();
	}

	void render_tzglp_previewer()
	{
		tzglp.render();
	}

	void render_ogl_info()
	{
		oglinfo.render();
	}

	void render_sentinel_tracker()
	{
		textracker.render();
	}

	void render_object_tracker()
	{
		ImGui::Begin("tz::gl::Object Tracker");
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

	void render_buffer_tracker()
	{
		tracker.render();
	}

	void render_menu()
	{
		static bool track_objects = false;
		if(ImGui::BeginMainMenuBar())
		{
			static bool wireframe = false;
			static bool cull_backfaces = true;
			if(ImGui::BeginMenu("Topaz 2"))
			{
				static bool do_assert = false;
				static bool do_hard_assert = false;
				ImGui::MenuItem("Test Assert", nullptr, &do_assert);
				ImGui::MenuItem("Test Hard Assert", nullptr, &do_hard_assert);
				ImGui::MenuItem("ImGui Demo", nullptr, &show_demo_window);
				if(do_assert)
				{
					topaz_assert(false, "Test Assert");
					do_assert = false;
				}
				if(do_hard_assert)
				{
					topaz_hard_assert(false, "Test Hard Assert");
					do_hard_assert = false;
				}
				ImGui::EndMenu();
			}

			if(ImGui::BeginMenu("tz"))
			{
				ImGui::EndMenu();
			}

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

			if(!windows.empty() && ImGui::BeginMenu(tz::get().window().get_title()))
			{
				for(const auto& window_ptr : windows)
				{
					ImGui::MenuItem(window_ptr->get_name(), nullptr, &window_ptr->visible);
				}
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();

			if(track_objects)
			{
				render_object_tracker();
			}

			if(tracker.visible)
			{
				tracker.render();
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
		}
	}

	void update()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if(show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		render_menu();

		for(auto& window_ptr : windows)
		{
			DebugWindow& wnd = *window_ptr;
			if(wnd.visible)
				wnd.render();
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
}