#ifndef TOPAZ_BUTTON_HPP
#define TOPAZ_BUTTON_HPP

#include "graphics/gui/widget.hpp"
#include "graphics/gui/display.hpp"
#include "utility/functional.hpp"

class Button : public GUIWidget
{
public:
    Button(Vector2I position_local_pixel_space, Vector2I dimensions_local_pixel_space, Font font, Vector3F text_colour, std::string text, Vector3F default_colour, Vector3F pressed_colour, float drop_shadow_size = 0.02f, GUI* parent = nullptr, std::initializer_list<GUI*> children = {});
    virtual void update() override;
    virtual void on_mouse_click() override;
    virtual void on_mouse_release() override;
    virtual void on_mouse_enter() override;
    virtual void on_mouse_leave() override;

    // These functions are necessary and MUST remain public so that EVIL sfinae hackery in window.inl works properly. This is the most disgusting practice used in the code base.
    Listener* get_key_listener(){return GUIWidget::get_key_listener();}
    Listener* get_mouse_listener(){return GUIWidget::get_mouse_listener();}
    void set_callback(tz::utility::functional::ButtonCallbackFunction callback);
    const tz::utility::functional::ButtonCallbackFunction& get_callback() const;
private:
    Panel drop_shadow;
    Panel background;
    Label text;
    Vector3F default_colour;
    Vector3F pressed_colour;
    float drop_shadow_size;
    tz::utility::functional::ButtonCallbackFunction callback;
};

#endif //TOPAZ_BUTTON_HPP
