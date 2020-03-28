#include "gl/tz_imgui/ogl_info.hpp"
#include "core/tz_glad/glad_context.hpp"
#include <string>
#include <vector>

namespace tz::ext::imgui::gl
{
    OpenGLInfoWindow::OpenGLInfoWindow(): ImGuiWindow("OpenGL Info"){}

    void OpenGLInfoWindow::render()
    {
        ImGui::Begin(this->get_name(), &this->visible);
        {
            // Get a bunch of info vars
            GLint major, minor;
            glGetIntegerv(GL_MAJOR_VERSION, &major);
            glGetIntegerv(GL_MINOR_VERSION, &minor);
            const GLubyte* full_version_string = glGetString(GL_VERSION);
            const GLubyte* vendor = glGetString(GL_VENDOR);
            const GLubyte* renderer = glGetString(GL_RENDERER);
            const GLubyte* glsl_version = glGetString(GL_SHADING_LANGUAGE_VERSION);

            GLint context_profile;
            glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &context_profile);
            std::string profile_name = "";
            // Account for nvidia driver bug which may wrongly have context_profile be zero. In this case, we just assume Core.
            if(context_profile == 0)
            {
                profile_name += "Core";
            }
            else
            {
                // Unaffected by nvidia bug, do as normal.
                if(context_profile & GL_CONTEXT_CORE_PROFILE_BIT)
                {
                    profile_name += "Core";
                }
                else if(context_profile & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT)
                {
                    profile_name += "Compatibility";
                }
                else
                {
                    profile_name += "Unknown";
                }
            }
            profile_name += " Profile";

            GLint context_flags;
            std::vector<const char*> flags;
            glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
            if(context_flags == 0)
            {
                // Nvidia bug again! If we're TOPAZ_DEBUG we'll just add debug context and no-error in Release.
                #if TOPAZ_DEBUG
                    flags.push_back("Debug Context");
                #else
                    flags.push_back("No Error (GL Errors replaced by UB)");
                #endif
            }
            else
            {
                // Do as normal.
                if(context_flags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT)
                {
                    flags.push_back("Forward Compatible Context");
                }
                if(context_flags & GL_CONTEXT_FLAG_DEBUG_BIT)
                {
                    flags.push_back("Debug Context");
                }
                if(context_flags & GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT)
                {
                    flags.push_back("Robust Access Context (Invalid accesses return 0)");
                }
                if(context_flags & GL_CONTEXT_FLAG_NO_ERROR_BIT)
                {
                    flags.push_back("No Error (GL Errors replaced by UB)");
                }
            }

            // Display them
            ImGui::Text("OpenGL Major/Minor Version: %d.%d", major, minor);
            ImGui::Text("OpenGL Full Version: %s", full_version_string);
            ImGui::Text("GLSL Version: %s", glsl_version);
            
            ImGui::Spacing();
            ImGui::Text("Graphics Vendor: %s", vendor);
            ImGui::Text("Graphics Renderer: %s", renderer);
            
            ImGui::Spacing();
            ImGui::Text("OpenGL Context Type: %s", profile_name.c_str());
            if(!flags.empty())
            {
                ImGui::Text("Context Flags:");
                ImGui::Indent();
                for(const char* flag_name : flags)
                {
                    ImGui::BulletText("%s", flag_name);
                }
                ImGui::Unindent();
            }
        }

        ImGui::End();
    }
}