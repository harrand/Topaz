#ifndef TOPAZ_TEXTFIELD_HPP
#define TOPAZ_TEXTFIELD_HPP
#include "graphics/gui/display.hpp"
#include "graphics/gui/widget.hpp"

class TextField : public GUIWidget
{
public:
    TextField(Vector2I position_local_pixel_space, Vector2I dimensions_local_pixel_space, Font font, Vector3F text_colour, std::string text, Vector3F background_colour, GUI* parent = nullptr, std::initializer_list<GUI*> children = {});
    virtual void update() override;
    // These functions are necessary and MUST remain public so that EVIL sfinae hackery in window.inl works properly. This is the most disgusting practice used in the code base.
    Listener* get_key_listener(){return GUIWidget::get_key_listener();}
    Listener* get_mouse_listener(){return GUIWidget::get_mouse_listener();}
private:
    virtual void on_key_down(const std::string& keyname) override;
    virtual void on_key_up(const std::string& keyname) override;
    //virtual void on_mouse_click() override
    Panel background;
    Label text;
    Vector3F background_colour;
    Vector3F text_colour;
};

#endif //TOPAZ_TEXTFIELD_HPP
