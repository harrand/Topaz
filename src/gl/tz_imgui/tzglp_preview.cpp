#include "gl/tz_imgui/tzglp_preview.hpp"
#include "gl/modules/ssbo.hpp"
#include "gl/modules/ubo.hpp"

namespace tz::ext::imgui::gl
{
    TZGLPPreview::TZGLPPreview(): ImGuiWindow("TZGLP Shader Source Previewer"), preprocessors(), show_module_configuration(false), dummy_object(nullptr){}

    TZGLPPreview::~TZGLPPreview()
    {
        if(this->dummy_object != nullptr)
            delete this->dummy_object;
    }

    std::size_t TZGLPPreview::add_preprocessor()
    {
        this->preprocessors.emplace_back("");
        return this->preprocessors.size() - 1;
    }

    void TZGLPPreview::render()
    {
        if(this->dummy_object == nullptr)
        {
            this->dummy_object = new tz::gl::Object;
        }
        ImGui::Begin(this->get_name());
        ImGui::TextWrapped("TZGLP stands for the Topaz GL Preprocessor. Shader Preprocessors can have at least zero modules, which affect the way it preprocesses GLSL source.");
        ImGui::Spacing();
        static int slider_choice = 0;
        if(ImGui::Button("Add preprocessor"))
        {
            this->add_preprocessor();
        }
        if(!this->preprocessors.empty())
        {
            ImGui::SliderInt("Preprocessor", &slider_choice, 0, this->preprocessors.size() - 1, "ID %d");
            if(ImGui::Button("Configure Modules"))
            {
                this->show_module_configuration = !this->show_module_configuration;
            }
        }

        static std::string input_source_data;
        auto text_resize_callback = [](ImGuiInputTextCallbackData* data)->int
        {
            if(data->EventFlag == ImGuiInputTextFlags_CallbackResize)
            {
                std::string* str = static_cast<std::string*>(data->UserData);
                topaz_assert(str->c_str() == data->Buf, "Uh oh");
                str->resize(data->BufTextLen);
                data->Buf = str->data();
            }
            return 0;
        };
        
        static std::string output_text;
        ImGui::Text("Modules consist of #include, #static_print, #ssbo, #ubo and more...");
        ImGui::InputTextMultiline("Shader Source", input_source_data.data(), input_source_data.size(), ImVec2(0, 0), ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_AllowTabInput, text_resize_callback, &input_source_data);
        if(!this->preprocessors.empty() && ImGui::Button("Invoke Preprocessor"))
        {
            output_text = this->preprocess(slider_choice, input_source_data);
        }
        if(!output_text.empty())
        {
            ImGui::Spacing();
            ImGui::Text("Output");
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "%s", output_text.c_str());
        }

        ImGui::End();
        if(this->show_module_configuration)
        {
            render_module_configuration(slider_choice);
        }
    }

    std::string TZGLPPreview::preprocess(std::size_t preprocessor_id, std::string source)
    {
        tz::gl::ShaderPreprocessor& proc = this->preprocessors[preprocessor_id];
        proc.set_source(source);
        proc.preprocess();
        return proc.result();
    }

    void TZGLPPreview::render_module_configuration(std::size_t preprocessor_id)
    {
        ImGui::Begin("TZGLP Module Configuration");
        ImGui::Text("Preprocessor ID: %zu", preprocessor_id);
        ImGui::Spacing();
        tz::gl::ShaderPreprocessor& proc = this->preprocessors[preprocessor_id];

        for(std::size_t i = 0; i < this->preprocessor_modules[preprocessor_id].size(); i++)
        {
            ImGui::Text("Module %zu = %s", i, get_supported_module_name(this->preprocessor_modules[preprocessor_id][i]));
        }
        static int t = 0;
        for(int i = 0; i < static_cast<int>(SupportedModule::Count); i++)
        {
            ImGui::RadioButton(get_supported_module_name(static_cast<SupportedModule>(i)), &t, i);
        }
        if(ImGui::Button("Add"))
        {
            SupportedModule choice = static_cast<SupportedModule>(t);
            switch(choice)
            {
                case SupportedModule::SSBOModule:
                    proc.emplace_module<tz::gl::p::SSBOModule>(this->dummy_object);
                break;
                case SupportedModule::UBOModule:
                    proc.emplace_module<tz::gl::p::UBOModule>(this->dummy_object);
                break;
            }
            this->preprocessor_modules[preprocessor_id].push_back(choice);
        }
        ImGui::End();
    }
}