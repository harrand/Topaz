#include "gl/tz_imgui/imgui_context.hpp"
#include "imgui.h"
#include "gl/tz_imgui/imgui_impl_opengl3.h"
#include "gl/tz_imgui/imgui_impl_glfw.h"
#include "glfw/glfw3.h"

namespace tz::ext::imgui
{
    void set_window_impl(GLFWwindow* wnd)
    {
        window = wnd;
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

    void update()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        static bool show_demo_window = true;
        ImGui::ShowDemoWindow(&show_demo_window);

        for(auto& window_ptr : windows)
        {
            ImGuiWindow& wnd = *window_ptr;
            wnd.render();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}