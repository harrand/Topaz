#include "graphics/gui/button.hpp"

Button::Button(Vector2I position_local_pixel_space, Vector2I dimensions_local_pixel_space, Font font, Vector3F text_colour, std::string text, Vector3F default_colour, Vector3F pressed_colour, float drop_shadow_size, GUI* parent, std::initializer_list<GUI*> children): GUIWidget(position_local_pixel_space, dimensions_local_pixel_space, true, false, parent, children), drop_shadow({}, dimensions_local_pixel_space, Vector4F{default_colour * 0.25f, 1.0f}, this), background({}, {}, Vector4F{default_colour, 1.0f}, &this->drop_shadow), text(Vector2I{}, font, text_colour, text, {}, &this->background), default_colour(default_colour), pressed_colour(pressed_colour), drop_shadow_size(drop_shadow_size), callback(nullptr)
{
    this->add_child(&this->drop_shadow);
    this->drop_shadow.add_child(&this->text);
    this->drop_shadow.add_child(&this->background);
}

void Button::update()
{
    int text_height_pixels = this->text.get_height();
    this->text.set_local_position_pixel_space((this->drop_shadow.get_local_position_pixel_space() * -1) + Vector2I(0, (this->drop_shadow.get_height() - text_height_pixels) / 2));
    this->set_local_dimensions_pixel_space({this->text.get_width(), this->text.get_height()});
    this->drop_shadow.set_local_dimensions_normalised_space({1.0f + this->drop_shadow_size, 1.0f + this->drop_shadow_size});
    //this->drop_shadow.set_local_dimensions_pixel_space({this->drop_shadow.get_width() + this->drop_shadow_size, this->drop_shadow.get_height() + this->drop_shadow_size});
    int drop_shadow_size_pixels = static_cast<int>(this->drop_shadow.get_width() * this->drop_shadow_size);
    this->background.set_local_position_pixel_space({0, drop_shadow_size_pixels});
    this->background.set_local_dimensions_pixel_space({this->drop_shadow.get_width() - drop_shadow_size_pixels, this->drop_shadow.get_height() - drop_shadow_size_pixels});
    /*
    Vector2F background_pos{Vector2F{static_cast<float>(this->drop_shadow.get_width()), static_cast<float>(this->drop_shadow.get_height())} * (1.0f - this->drop_shadow_size)};
    this->background.set_local_dimensions_pixel_space({static_cast<int>(background_pos.x), static_cast<int>(background_pos.y)});
         */
    GUIWidget::update();
}

void Button::on_mouse_click()
{
    this->background.set_colour({this->pressed_colour, 1.0f});
    if(this->callback != nullptr)
        this->callback();
}

void Button::on_mouse_release()
{
    this->background.set_colour({this->default_colour, 1.0f});
}

void Button::on_mouse_enter()
{
    this->background.set_colour({this->default_colour * 1.5f, 1.0f});
}

void Button::on_mouse_leave()
{
    this->background.set_colour({this->default_colour, 1.0f});
}

using namespace tz::utility::functional;

void Button::set_callback(ButtonCallbackFunction callback)
{
    this->callback = callback;
}

const ButtonCallbackFunction& Button::get_callback() const
{
    return this->callback;
}