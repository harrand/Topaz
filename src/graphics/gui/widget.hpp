#ifndef TOPAZ_GUI_WIDGET_HPP
#define TOPAZ_GUI_WIDGET_HPP

#include "core/listener.hpp"
#include "graphics/gui/gui.hpp"

class GUIWidget;

class GUIWidgetKeyListener : public KeyListener
{
public:
    GUIWidgetKeyListener(GUIWidget& widget);
    GUIWidgetKeyListener(GUIWidget& widget, Window& window);
    virtual void on_key_pressed(const std::string& keyname) override;
    virtual void on_key_released(const std::string& keyname) override;
private:
    GUIWidget& widget;
};

class GUIWidget : public GUI
{
public:
    GUIWidget(Vector2I position_local_pixel_space, Vector2I dimensions_local_pixel_space, bool mouse_sensitive, bool key_sensitive, GUI* parent = nullptr, std::initializer_list<GUI*> children = {});
    ~GUIWidget();
    friend class Window;
    friend class GUIWidgetKeyListener;

    virtual void update() override;
    virtual bool key_sensitive() const override;
    virtual bool mouse_sensitive() const override;
    virtual void on_mouse_enter(){}
    virtual void on_mouse_leave(){}
    virtual void on_mouse_reside(){}
    virtual void on_mouse_click(){}
    virtual void on_mouse_release(){}
    virtual void on_key_down([[maybe_unused]] const std::string& keyname){}
    virtual void on_key_up([[maybe_unused]] const std::string& keyname){}

    // These functions are necessary and MUST remain public so that EVIL sfinae hackery in window.inl works properly. This is the most disgusting practice used in the code base.
    Listener* get_key_listener();
    Listener* get_mouse_listener();
private:
    void on_key_typed_unconditional(const std::string& keyname);
    void on_key_released_unconditional(const std::string& keyname);

    std::optional<GUIWidgetKeyListener> key_listener;
    std::optional<MouseListener> mouse_listener;
    bool mouse_inside;
    bool mouse_just_clicked;
};


#endif //TOPAZ_GUI_WIDGET_HPP
