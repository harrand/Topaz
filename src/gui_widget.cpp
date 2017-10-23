#include "gui_widget.hpp"

Button::Button(float x, float y, Vector4F colour, std::optional<Vector4F> background_colour, std::optional<Vector3F> text_border_colour, Font font, const std::string& text, const Shader& shader, MouseListener& mouse_listener): TextLabel(x, y, colour, background_colour, text_border_colour, font, text, shader), mouse_listener(mouse_listener), just_clicked(false), just_moused_over(false), on_mouse_over(nullptr), on_mouse_click(nullptr){}

void Button::update()
{
	if(this->clicked_on() && this->on_mouse_click != nullptr && !this->just_clicked && this->has_window_parent() && !this->covered())
	{
		// if clicked on properly, run the mouse_click command, set it as just clicked and make it the focus of the window ancestor
		this->find_window_parent()->set_focused_child(this);
		this->on_mouse_click->operator()({});
		this->just_clicked = true;
	}
	else if(!this->clicked_on())
		this->just_clicked = false;
	// if click mouse button is down but this is not moused over, make sure its not focused
	if(this->mouse_listener.is_left_clicked() && !this->moused_over() && this->focused())
		this->find_window_parent()->set_focused_child(nullptr);
	TextLabel::update();
}

bool Button::focused() const
{
	if(!this->has_window_parent())
		return false;
	return this->find_window_parent()->get_focused_child() == this;
}

Command* Button::get_on_mouse_over() const
{
	return this->on_mouse_over;
}

Command* Button::get_on_mouse_click() const
{
	return this->on_mouse_click;
}

void Button::set_on_mouse_over(Command* cmd)
{
	this->on_mouse_over = cmd;
}

void Button::set_on_mouse_click(Command* cmd)
{
	this->on_mouse_click = cmd;
}

bool Button::moused_over() const
{
	Vector2F mouse_pos = this->mouse_listener.get_mouse_pos();
	bool x_aligned = mouse_pos.get_x() >= (this->get_window_pos_x() - this->width) && mouse_pos.get_x() <= (this->get_window_pos_x() + this->width);
	bool y_aligned = mouse_pos.get_y() >= (this->find_window_parent()->get_height() - this->get_window_pos_y() - this->height) && mouse_pos.get_y() <= ((this->find_window_parent()->get_height() - this->get_window_pos_y() + this->height));
	return x_aligned && y_aligned;
}

bool Button::clicked_on() const
{
	// need to take into account the location where the left click was pressed to prevent dragging from firing off the button.
	Vector2F mouse_pos = this->mouse_listener.get_left_click_location();
	bool x_aligned = mouse_pos.get_x() >= (this->get_window_pos_x() - this->width) && mouse_pos.get_x() <= (this->get_window_pos_x() + this->width);
	bool y_aligned = mouse_pos.get_y() >= (this->find_window_parent()->get_height() - this->get_window_pos_y() - this->height) && mouse_pos.get_y() <= ((this->find_window_parent()->get_height() - this->get_window_pos_y() + this->height));
	return this->mouse_listener.is_left_clicked() && x_aligned && y_aligned;
}