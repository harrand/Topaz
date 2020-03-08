#include "gl/tz_imgui/imgui_context.hpp"
#include "gl/object.hpp"
#include "gl/tz_imgui/imgui_impl_opengl3.h"
#include "gl/tz_imgui/imgui_impl_glfw.h"
#include "glfw/glfw3.h"
#include "core/debug/assert.hpp"

// Diagnostics
#include <vector>

namespace tz::ext::imgui
{
    static std::vector<std::unique_ptr<ImGuiWindow>> windows;
    static tz::gl::Object* obj = nullptr;

    void set_window_impl(GLFWwindow* wnd)
    {
        window = wnd;
    }

    void track_object(tz::gl::Object* object)
    {
        obj = object;
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

    ImGuiWindow& add_window(std::unique_ptr<ImGuiWindow> wnd)
    {
        windows.push_back(std::move(wnd));
        return *windows.back();
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
                                ImGui::Text("Terminal: %d", buf->is_terminal());
                                ImGui::Text("Mapped: %d", buf->is_mapped());
                                ImGui::TreePop();
                            }
                        }
                        ImGui::TreePop();
                    }
                }
                ImGui::End();
    }

    void render_menu()
    {
        static bool track_objects = false;
        if(ImGui::BeginMainMenuBar())
        {

            if(ImGui::BeginMenu("tz2"))
            {
                // ooga booga!
                static bool do_assert = false;
                ImGui::MenuItem("Test Assert", nullptr, &do_assert);
                if(do_assert)
                {
                    topaz_assert(false, "Test Assert");
                    do_assert = false;
                }
                ImGui::EndMenu();
            }

            if(ImGui::BeginMenu("tz::core"))
            {
                ImGui::EndMenu();
            }

            if(ImGui::BeginMenu("tz::gl"))
            {
                ImGui::MenuItem("gl::Object Tracking", nullptr, &track_objects);
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();

            if(track_objects)
            {
                render_object_tracker();
            }
        }
    }

    void update()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        static bool show_demo_window = true;
        ImGui::ShowDemoWindow(&show_demo_window);

        render_menu();

        for(auto& window_ptr : windows)
        {
            ImGuiWindow& wnd = *window_ptr;
            wnd.render();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}