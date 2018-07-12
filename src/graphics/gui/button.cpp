#include "button.hpp"

Button::Button(Vector2I position_local_pixel_space, Vector2I dimensions_local_pixel_space, Font font, Vector3F text_colour, std::string text, Vector3F default_colour, Vector3F pressed_colour, int drop_shadow_size, GUI* parent, std::initializer_list<GUI*> children): GUIWidget(position_local_pixel_space, dimensions_local_pixel_space, true, false, parent, children), drop_shadow(position_local_pixel_space, dimensions_local_pixel_space, Vector4F{default_colour * 0.25f, 1.0f}, this), background({}, {}, Vector4F{default_colour, 1.0f}, &this->drop_shadow), text(Vector2I{}, font, text_colour, text, {}, &this->background), default_colour(default_colour), pressed_colour(pressed_colour), drop_shadow_size(drop_shadow_size), callback(nullptr)
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
    this->drop_shadow.set_local_dimensions_normalised_space({1.0f, 1.0f});
    this->drop_shadow.set_local_dimensions_pixel_space({this->drop_shadow.get_width() + this->drop_shadow_size, this->drop_shadow.get_height() + this->drop_shadow_size});
    this->background.set_local_position_pixel_space({0, this->drop_shadow_size * 2});
    this->background.set_local_dimensions_pixel_space(Vector2I{this->drop_shadow.get_width(), this->drop_shadow.get_height()} - Vector2I{this->drop_shadow_size, this->drop_shadow_size});
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