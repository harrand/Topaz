#include <physics/boundary.hpp>
#include "gui_widget.hpp"

GUIWidgetKeyListener::GUIWidgetKeyListener(GUIWidget& widget): KeyListener(), widget(widget){}
GUIWidgetKeyListener::GUIWidgetKeyListener(GUIWidget& widget, Window& window): KeyListener(window), widget(widget){}

void GUIWidgetKeyListener::on_key_pressed(const std::string& keyname)
{
    this->widget.on_key_typed_unconditional(keyname);
}

void GUIWidgetKeyListener::on_key_released(const std::string& keyname)
{
    this->widget.on_key_released_unconditional(keyname);
}

GUIWidget::GUIWidget(Vector2I position_local_pixel_space, Vector2I dimensions_local_pixel_space, bool mouse_sensitive, bool key_sensitive, GUI* parent, std::initializer_list<GUI*> children): GUI(position_local_pixel_space, dimensions_local_pixel_space, parent, children), key_listener({*this}), mouse_listener(MouseListener()), mouse_inside(false), mouse_just_clicked(false)
{
    // it is upto the parent to update this widget's mouse and key listener.
    if(!mouse_sensitive)
        this->mouse_listener = std::nullopt;
    if(!key_sensitive)
        this->key_listener = std::nullopt;
}

void GUIWidget::update()
{
    GUI::update();
    if(this->mouse_sensitive())
    {
        // handle mouse events.
        Vector3F position_floating{static_cast<float>(this->position_local_pixel_space.x), static_cast<float>(this->position_local_pixel_space.y), 0.0f};
        Vector3F dimensions_floating{static_cast<float>(this->dimensions_local_pixel_space.x), static_cast<float>(this->dimensions_local_pixel_space.y), 0.0f};
        AABB pixel_boundary{position_floating, position_floating + dimensions_floating};
        Vector3F mouse_position{this->mouse_listener->get_mouse_position(), 0.0f};
        //std::cout << "mouse pos = " << mouse_position << "\n";
        if(pixel_boundary.intersects(mouse_position))
        {
            // the mouse is inside this GUI element, invoke GUIWidget::on_mouse_reside().
            this->on_mouse_reside();
            if(this->mouse_listener->is_left_clicked())
            {
                if(!this->mouse_just_clicked)
                {
                    this->on_mouse_click();
                    this->mouse_just_clicked = true;
                }
            }
            else if(this->mouse_just_clicked)
            {
                this->on_mouse_release();
                this->mouse_just_clicked = false;
            }
            // if it previously wasn't, then invoke GUIWidget::on_mouse_enter()
            if(!this->mouse_inside)
            {
                this->on_mouse_enter();
                this->mouse_inside = true;
            }
        }
        else if(this->mouse_inside)
        {
            this->on_mouse_leave();
            this->mouse_inside = false;
        }

    }
    if(this->key_sensitive())
    {
        // handle key events.
    }
}

bool GUIWidget::key_sensitive() const
{
    return this->key_listener.has_value();
}

bool GUIWidget::mouse_sensitive() const
{
    return this->mouse_listener.has_value();
}

Listener* GUIWidget::get_key_listener()
{
    if(this->key_listener.has_value())
        return &this->key_listener.value();
    else
        return nullptr;
}

Listener* GUIWidget::get_mouse_listener()
{
    if(this->mouse_listener.has_value())
        return &this->mouse_listener.value();
    else
        return nullptr;
}

void GUIWidget::on_key_typed_unconditional(const std::string& keyname)
{
    if(this->mouse_inside)
        this->on_key_down(keyname);
}

void GUIWidget::on_key_released_unconditional(const std::string& keyname)
{
    if(this->mouse_inside)
        this->on_key_up(keyname);
}