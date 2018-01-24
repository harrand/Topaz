#ifndef GUI_WIDGET_HPP
#define GUI_WIDGET_HPP
#include "gui_display.hpp"

/**
* Just like a TextLabel, but also is mouse-sensitive and pressable to execute a command.
*/
class Button : public TextLabel
{
public:
	Button(float x, float y, Vector4F colour, std::optional<Vector4F> background_colour, std::optional<Vector3F> text_border_colour, Font font, const std::string& text, Shader& shader, MouseListener& mouse_listener);
	Button(const Button& rhs) = default;
	Button(Button&& move) = default;
	~Button() = default;
	Button& operator=(const Button& rhs) = default;
	
	virtual void update() override;
	virtual bool focused() const override;
	virtual bool is_mouse_sensitive() const override{return true;}
	Command* get_on_mouse_over() const;
	Command* get_on_mouse_click() const;
	void set_on_mouse_over(Command* cmd);
	void set_on_mouse_click(Command* cmd);
	bool moused_over() const;
	bool clicked_on() const;
protected:
	MouseListener& mouse_listener;
	bool just_clicked, just_moused_over;
private:
	Command* on_mouse_over;
	Command* on_mouse_click;
};

class BoolBox : public Panel
{
public:
	BoolBox(float x, float y, float width, float height, Vector4F colour_on, Vector4F colour_off, Shader& shader, MouseListener& mouse_listener, bool ticked = false);
	BoolBox(const BoolBox& copy) = default;
	BoolBox(BoolBox&& move) = default;
	~BoolBox() = default;
	BoolBox& operator=(const BoolBox& rhs) = default;
	
	virtual void update() override;
	virtual bool focused() const override;
	virtual bool is_mouse_sensitive() const override{return true;}
	bool moused_over() const;
	bool clicked_on() const;
	
	const Vector4F& get_colour_on() const;
	const Vector4F& get_colour_off() const;
	Vector4F get_colour() const;
	
	bool value;
protected:
	Vector4F colour_on, colour_off;
	MouseListener& mouse_listener;
	bool just_clicked, just_moused_over;
};

namespace tz::ui
{
	bool moused_over(const GUI* gui, Vector2F mouse_position);
	bool left_clicked(const GUI* gui, const MouseListener& mouse_listener);
	bool right_clicked(const GUI* gui, const MouseListener& mouse_listener);
}

#endif