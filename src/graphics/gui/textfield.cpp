#include <thread>
#include "graphics/gui/textfield.hpp"

TextField::TextField(Vector2I position_local_pixel_space, Vector2I dimensions_local_pixel_space, Font font, Vector3F text_colour, std::string text, Vector3F background_colour, GUI* parent, std::initializer_list<GUI*> children): GUIWidget(position_local_pixel_space, dimensions_local_pixel_space, false, true, parent, children), background(position_local_pixel_space, dimensions_local_pixel_space, Vector4F{background_colour, 1.0f}, this), text(Vector2I{}, font, text_colour, text, {}, &this->background), background_colour(background_colour), text_colour(text_colour)
{
    this->add_child(&this->background);
    this->background.add_child(&this->text);
}

void TextField::update()
{
    GUIWidget::update();
    int text_height_pixels = this->text.get_height();
    this->text.set_local_position_pixel_space((this->background.get_local_position_pixel_space() * -1) + Vector2I(0, (this->background.get_height() - text_height_pixels) / 2));
    this->set_local_dimensions_pixel_space({this->text.get_width(), this->text.get_height()});
    this->background.set_local_dimensions_normalised_space({1.0f, 1.0f});
}

void TextField::on_key_down(const std::string& keyname)
{
    if(keyname == "Backspace")
    {
        std::string text = this->text.get_text();
        if(text.length() <= 1)
            text = " ";
        else
            text.pop_back();
        this->text.set_text(text);
        return;
    }
    SDL_Keymod mod = SDL_GetModState();
    std::string key = keyname;
    if (mod & KMOD_SHIFT || mod & KMOD_CAPS)
        key = tz::utility::string::to_upper(key);
    else
        key = tz::utility::string::to_lower(key);
    if(keyname == "Space")
        key = " ";
    if(keyname != "Left Shift" && keyname != "Right Shift" && keyname != "Left Ctrl" && keyname != "Right Ctrl" && keyname != "CapsLock")
        this->text.set_text(this->text.get_text() + key);
}

void TextField::on_key_up([[maybe_unused]] const std::string& keyname)
{

}

