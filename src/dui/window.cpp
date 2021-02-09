#include "dui/window.hpp"
#include "gl/object.hpp"
#include "dui/imgui_impl_opengl3.h"
#include "dui/imgui_impl_glfw.h"
#include "glfw/glfw3.h"
#include "core/debug/assert.hpp"
#include "core/tz.hpp"
#include "dui/module_tabs/core_tab.hpp"
#include "dui/module_tabs/gl_tab.hpp"

// Diagnostics
#include <vector>

namespace tz::dui
{
	static std::vector<std::unique_ptr<DebugWindow>> windows;

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
		gl::track_object(object);
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

	void render_menu()
	{
		if(ImGui::BeginMainMenuBar())
		{
			core::draw_tab();
			gl::draw_tab();

			if(!windows.empty() && ImGui::BeginMenu(tz::get().window().get_title()))
			{
				for(const auto& window_ptr : windows)
				{
					ImGui::MenuItem(window_ptr->get_name(), nullptr, &window_ptr->visible);
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}

	void update()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

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