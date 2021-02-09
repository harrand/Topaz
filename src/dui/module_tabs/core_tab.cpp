#include "core/debug/assert.hpp"
#include "dui/module_tabs/core_tab.hpp"
#include "dui/window.hpp"

namespace tz::dui::core
{
    static bool show_demo_window = false;

    void draw_tab()
    {
        if(show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

        if(ImGui::BeginMenu("tz"))
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
    }
}