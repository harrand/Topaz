#include "core/core.hpp"
#include "gl/frame.hpp"
#include "dui/imgui_context.hpp"

class InputDemoWindow : public tz::ext::imgui::ImGuiWindow
{
public:
    InputDemoWindow(): tz::ext::imgui::ImGuiWindow("Topaz PC Input Demo"){}
    
    void handle_key_press(tz::input::KeyPressEvent e);
    void handle_char_type(tz::input::CharPressEvent e);
    void handle_mouse_update(tz::input::MouseUpdateEvent e);
    void handle_mouse_click(tz::input::MouseClickEvent e);

    virtual void render() override;
private:
    tz::input::KeyPressEvent last_key_press;
    tz::input::CharPressEvent last_char_type;
    tz::input::MouseUpdateEvent last_mouse_update;
    tz::input::MouseClickEvent last_mouse_click;
};

int main()
{
    tz::core::initialise("Topaz Window Demo");
    {
        tz::core::IWindow& window = tz::core::get().window();
        window.register_this();
        InputDemoWindow& input = tz::ext::imgui::emplace_window<InputDemoWindow>();

        window.emplace_custom_key_listener(
        [&input](tz::input::KeyPressEvent e)
        {
            input.handle_key_press(e);
        });

        window.emplace_custom_type_listener(
        [&input](tz::input::CharPressEvent e){
            input.handle_char_type(e);
        });

        window.emplace_custom_mouse_listener(
        [&input](tz::input::MouseUpdateEvent e)
        {
            input.handle_mouse_update(e);
        },
        [&input](tz::input::MouseClickEvent e)
        {
            input.handle_mouse_click(e);
        });

        while(!window.is_close_requested())
        {
            window.get_frame()->clear();
            // Do stuff.
            tz::core::update();
            window.update();
        }
        
    }
    tz::core::terminate();
    return 0;
}

void InputDemoWindow::handle_key_press(tz::input::KeyPressEvent e)
{
    this->last_key_press = e;
}

void InputDemoWindow::handle_char_type(tz::input::CharPressEvent e)
{
    this->last_char_type = e;
}

void InputDemoWindow::handle_mouse_update(tz::input::MouseUpdateEvent e)
{
    this->last_mouse_update = e;
}

void InputDemoWindow::handle_mouse_click(tz::input::MouseClickEvent e)
{
    this->last_mouse_click = e;
}

void InputDemoWindow::render()
{
    ImGui::Begin("Topaz PC Input Demo");
    if(ImGui::CollapsingHeader("Keyboard", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Info about last key press.
        ImGui::Text("Last Key Press");
        {
            ImGui::Indent();
            ImGui::Text("Key: %d", this->last_key_press.key);
            ImGui::Text("Scancode: %d", this->last_key_press.scancode);
            ImGui::Text("Action: %d", this->last_key_press.action);
            ImGui::Text("Mods: %d", this->last_key_press.mods);
            ImGui::Text("Key Name: %s", this->last_key_press.get_key_name());
            ImGui::Unindent();
        }
        ImGui::Spacing();
        // Info about last char typed.
        ImGui::Text("Last Char Press");
        {
            ImGui::Indent();
            ImGui::Text("Codepoint: %ul", this->last_char_type.codepoint);
            ImGui::Text("Character: '%c'", static_cast<char>(this->last_char_type.get_char()));
            ImGui::Unindent();
        }
    }

    if(ImGui::CollapsingHeader("Mouse", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Info about last mouse update (moved around etc...)
        ImGui::Text("Last Mouse Update");
        {
            ImGui::Indent();
            ImGui::Text("Position X: %g", this->last_mouse_update.xpos);
            ImGui::Text("Position Y: %g", this->last_mouse_update.ypos);
            ImGui::Unindent();
        }
        ImGui::Spacing();
        // Info about last mouse click (any button, any state be it down or up)
        ImGui::Text("Last Mouse Click");
        {
            ImGui::Indent();
            ImGui::Text("Button: %d", this->last_mouse_click.button);
            ImGui::Text("Action: %d", this->last_mouse_click.action);
            ImGui::Text("Mods: %d", this->last_mouse_click.mods);
            ImGui::Unindent();
        }
    }
    ImGui::End();
}