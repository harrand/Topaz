#include "gui_widget.hpp"

Button::Button(float x, float y, Vector4F colour, std::optional<Vector4F> background_colour, std::optional<Vector3F> text_border_colour, Font font, const std::string& text, Shader& shader, MouseListener& mouse_listener): TextLabel(x, y, colour, background_colour, text_border_colour, font, text, shader), mouse_listener(mouse_listener), just_clicked(false), just_moused_over(false), on_mouse_over(nullptr), on_mouse_click(nullptr){}

void Button::update()
{
	if(this->on_mouse_click != nullptr)
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
	}
	if(this->on_mouse_over != nullptr)
	{
		if(this->moused_over() && !this->just_moused_over)
		{
			this->on_mouse_over->operator()({});
			this->just_moused_over = true;
		}
		else if(!this->moused_over())
			this->just_moused_over = false;
	}
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
	bool x_aligned = mouse_pos.x >= (this->get_window_pos_x() - this->width) && mouse_pos.x <= (this->get_window_pos_x() + this->width);
	bool y_aligned = mouse_pos.y >= (this->find_window_parent()->get_height() - this->get_window_pos_y() - this->height) && mouse_pos.y <= ((this->find_window_parent()->get_height() - this->get_window_pos_y() + this->height));
	return x_aligned && y_aligned;
}

bool Button::clicked_on() const
{
	// need to take into account the location where the left click was pressed to prevent dragging from firing off the button.
	Vector2F mouse_pos = this->mouse_listener.get_left_click_location();
	bool x_aligned = mouse_pos.x >= (this->get_window_pos_x() - this->width) && mouse_pos.x <= (this->get_window_pos_x() + this->width);
	bool y_aligned = mouse_pos.y >= (this->find_window_parent()->get_height() - this->get_window_pos_y() - this->height) && mouse_pos.y <= ((this->find_window_parent()->get_height() - this->get_window_pos_y() + this->height));
	return this->mouse_listener.is_left_clicked() && x_aligned && y_aligned;
}

Checkbox::Checkbox(float x, float y, float width, float height, Vector4F colour_on, Vector4F colour_off, Shader& shader, MouseListener& mouse_listener, bool ticked): Panel(x, y, width, height, colour_off, shader), value(ticked), colour_on(colour_on), colour_off(colour_off), mouse_listener(mouse_listener), just_clicked(false), just_moused_over(false), choice_parent(nullptr){}


void Checkbox::update()
{
	if(!this->hidden)
	{
		this->render_panel(this->get_colour());
		if(this->clicked_on() && !this->just_clicked && this->has_window_parent() && !this->covered())
		{
			// if clicked on properly, run the mouse_click command, set it as just clicked and make it the focus of the window ancestor
			this->find_window_parent()->set_focused_child(this);
			this->just_clicked = true;
			if(this->is_choice())
				this->choice_parent->set_choice(this);
			else
				this->value = !this->value;
		}
		else if(!this->clicked_on())
			this->just_clicked = false;
		if(this->moused_over() && !this->just_moused_over)
			this->just_moused_over = true;
		else if(!this->moused_over())
			this->just_moused_over = false;
		// if click mouse button is down but this is not moused over, make sure its not focused
		if(this->mouse_listener.is_left_clicked() && !this->moused_over() && this->focused())
			this->find_window_parent()->set_focused_child(nullptr);
	}
}

bool Checkbox::focused() const
{
	if(!this->has_window_parent())
		return false;
	return this->find_window_parent()->get_focused_child() == this;
}

bool Checkbox::moused_over() const
{
	return tz::ui::moused_over(this, this->mouse_listener.get_mouse_pos());
}

bool Checkbox::clicked_on() const
{
	return tz::ui::left_clicked(this, this->mouse_listener);
}

const Vector4F& Checkbox::get_colour_on() const
{
	return this->colour_on;
}

const Vector4F& Checkbox::get_colour_off() const
{
	return this->colour_off;
}

Vector4F Checkbox::get_colour() const
{
	return value ? this->colour_on : this->colour_off;
}

bool Checkbox::is_choice() const
{
	return this->choice_parent != nullptr;
}

Slider::Slider(float x, float y, float width, float height, Vector4F slider_colour, Vector4F background_colour, Vector2F slider_size, Shader& shader, MouseListener& mouse_listener, float position): Panel(x, y, width, height, background_colour, shader), position(position), slider_colour(slider_colour), slider_size(slider_size), mouse_listener(mouse_listener), just_clicked(false), just_moused_over(false){}

void Slider::update()
{
	if(!this->hidden)
	{
		this->render_panel(this->get_colour(), false);
		// Render bar of slider
		this->shader.value().get().bind();
		this->shader.value().get().set_uniform<bool>("has_texture", false);
		this->shader.value().get().set_uniform<bool>("has_background_colour", false);
		this->shader.value().get().set_uniform<bool>("has_text_border_colour", false);
		this->shader.value().get().set_uniform<Vector4F>("colour", this->slider_colour);
		Matrix4x4 projection;
		if(this->has_window_parent() && !this->use_proportional_positioning)
			projection = tz::ui::create_orthographic_gui_matrix(this);
		else
			projection = Matrix4x4::identity();
		this->shader.value().get().set_uniform<Matrix4x4>("model_matrix", projection * tz::transform::model(Vector3F(this->get_x() + (this->slider_size.x / 2) + (2.0f * this->position * this->get_width()), this->get_window_pos_y(), 0.0f), Vector3F(), Vector3F(this->slider_size, 0.0f)));
		this->shader.value().get().update();
		this->quad.render(false);
		GUI::update();
		
		if(this->clicked_on() && !this->just_clicked && this->has_window_parent() && !this->covered())
		{
			// if clicked on properly, run the mouse_click command, set it as just clicked and make it the focus of the window ancestor
			this->find_window_parent()->set_focused_child(this);
			this->just_clicked = true;
			// do stuff on click here.
			float mouse_distance = this->mouse_listener.get_left_click_location().x - this->get_x();
			position = 0.5f * mouse_distance / this->get_width();
			position = std::clamp(position, 0.0, 1.0);
		}
		else if(!this->clicked_on())
			this->just_clicked = false;
		if(this->moused_over() && !this->just_moused_over)
			this->just_moused_over = true;
		else if(!this->moused_over())
			this->just_moused_over = false;
		// if click mouse button is down but this is not moused over, make sure its not focused
		if(this->mouse_listener.is_left_clicked() && !this->moused_over() && this->focused())
			this->find_window_parent()->set_focused_child(nullptr);
	}
}

bool Slider::focused() const
{
	if(!this->has_window_parent())
		return false;
	return this->find_window_parent()->get_focused_child() == this;
}

bool Slider::moused_over() const
{
	return tz::ui::moused_over(this, this->mouse_listener.get_mouse_pos());
}

bool Slider::clicked_on() const
{
	return tz::ui::left_clicked(this, this->mouse_listener);
}

const Vector4F& Slider::get_slider_colour() const
{
	return this->slider_colour;
}

const Vector2F& Slider::get_slider_size() const
{
	return this->slider_size;
}

CheckboxChoice::CheckboxChoice(std::initializer_list<Checkbox*> boxes, Checkbox* initial_choice): boxes(boxes), choice(nullptr)
{
	for(auto& box : this->boxes)
		box->choice_parent = this;
	this->set_all(false);
	this->set_choice(initial_choice);
}

CheckboxChoice::CheckboxChoice(std::initializer_list<std::reference_wrapper<Checkbox>> boxes, Checkbox* initial_choice): boxes(), choice(nullptr)
{
	for(auto& reference_wrapper : boxes)
	{
		reference_wrapper.get().choice_parent = this;
		this->boxes.insert(&(reference_wrapper.get()));
	}
	this->set_all(false);
	this->set_choice(initial_choice);
}

bool CheckboxChoice::has_choice() const
{
	if(this->choice == nullptr)
		return false;
	return this->choice_in_scope(this->choice);
}

Checkbox* CheckboxChoice::get_choice() const
{
	return this->choice;
}

void CheckboxChoice::set_choice(Checkbox* choice)
{
	if(!this->choice_in_scope(choice))
	{
		tz::util::log::error("Tried to set a CheckboxChoice to a Checkbox not belonging to the set.");
		return;
	}
	this->choice = choice;
	this->set_all(false);
	this->choice->value = true;
}

const std::unordered_set<Checkbox*>& CheckboxChoice::get_bool_boxes() const
{
	return this->boxes;
}

bool CheckboxChoice::choice_in_scope(Checkbox* choice) const
{
	return this->boxes.find(choice) != this->boxes.end();
}

void CheckboxChoice::set_all(bool value)
{
	for(auto& box : this->boxes)
		box->value = value;
}

namespace tz::ui
{
	bool moused_over(const GUI* gui, Vector2F mouse_position)
	{
		bool x_aligned = mouse_position.x >= (gui->get_window_pos_x() - gui->get_width()) && mouse_position.x <= (gui->get_window_pos_x() + gui->get_width());
		bool y_aligned = mouse_position.y >= (gui->find_window_parent()->get_height() - gui->get_window_pos_y() - gui->get_height()) && mouse_position.y <= ((gui->find_window_parent()->get_height() - gui->get_window_pos_y() + gui->get_height()));
		return x_aligned && y_aligned;
	}
	
	bool left_clicked(const GUI* gui, const MouseListener& mouse_listener)
	{
		return tz::ui::moused_over(gui, mouse_listener.get_left_click_location()) && mouse_listener.is_left_clicked();
	}
	
	bool right_clicked(const GUI* gui, const MouseListener& mouse_listener)
	{
		return tz::ui::moused_over(gui, mouse_listener.get_right_click_location()) && mouse_listener.is_right_clicked();
	}
}