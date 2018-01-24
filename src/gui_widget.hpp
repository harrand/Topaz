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

class CheckBoxChoice;

class CheckBox : public Panel
{
public:
	CheckBox(float x, float y, float width, float height, Vector4F colour_on, Vector4F colour_off, Shader& shader, MouseListener& mouse_listener, bool ticked = false);
	CheckBox(const CheckBox& copy) = default;
	CheckBox(CheckBox&& move) = default;
	~CheckBox() = default;
	CheckBox& operator=(const CheckBox& rhs) = default;
	
	virtual void update() override;
	virtual bool focused() const override;
	virtual bool is_mouse_sensitive() const override{return true;}
	bool moused_over() const;
	bool clicked_on() const;
	
	const Vector4F& get_colour_on() const;
	const Vector4F& get_colour_off() const;
	Vector4F get_colour() const;
	
	friend class CheckBoxChoice;
	
	bool value;
protected:
	bool is_choice() const;

	Vector4F colour_on, colour_off;
	MouseListener& mouse_listener;
	bool just_clicked, just_moused_over;
	/**
	* choice_parent is handled purely by the friend-class CheckBoxChoice.
	*/
	CheckBoxChoice* choice_parent;
};

class Slider : public Panel
{
public:
	Slider(float x, float y, float width, float height, Vector4F slider_colour, Vector4F background_colour, Vector2F slider_size, Shader& shader, MouseListener& mouse_listener);
	Slider(const Slider& copy) = default;
	Slider(Slider&& move) = default;
	~Slider() = default;
	Slider& operator=(const Slider& rhs) = default;
	
	virtual void update() override;
	virtual bool focused() const override;
	virtual bool is_mouse_sensitive() const override{return true;}
	bool moused_over() const;
	bool clicked_on() const;
	
	double position;
private:
	Vector4F slider_colour;
	Vector2F slider_size;
	MouseListener& mouse_listener;
	bool just_clicked, just_moused_over;
};

/**
* Non-owning helper class to manage multiple CheckBoxes.
*/
class CheckBoxChoice
{
public:
	/**
	* Non-owning pointer initialisation.
	*/
	CheckBoxChoice(std::initializer_list<CheckBox*> boxes, CheckBox* initial_choice = nullptr);
	/**
	* Non-owning reference initialisation.
	*/
	CheckBoxChoice(std::initializer_list<std::reference_wrapper<CheckBox>> boxes, CheckBox* initial_choice = nullptr);
	/**
	* Copy constructor deleted.
	* This is because any two CheckBoxChoices may not share a single element, or "choice-fighting" will occur.
	* Choice-fighting is the phenomenon such that multiple CheckBoxChoices choose different boxes to truthify, causing the latter to always invalidate the former.
	*/
	CheckBoxChoice(const CheckBoxChoice& copy) = delete;
	/**
	* Move constructor implies the rvalue-reference parameter is about to go out-of-scope, meaning that it will not be available to cause choice-fighting.
	* For this reason, the move constructor is available.
	*/
	CheckBoxChoice(CheckBoxChoice&& move) = default;
	~CheckBoxChoice() = default;
	/**
	* Copy assignment operator is also deleted for the same reason; choice-fighting.
	*/
	CheckBoxChoice& operator=(const CheckBoxChoice& rhs) = delete;
	/**
	* Similarly to the move constructor, the move assignment-operator cannot induce choice-fighting so remains available for use.
	*/
	CheckBoxChoice& operator=(CheckBoxChoice&& rhs) = default;
	
	bool has_choice() const;
	CheckBox* get_choice() const;
	void set_choice(CheckBox* choice);
	const std::unordered_set<CheckBox*>& get_bool_boxes() const;
private:
	bool choice_in_scope(CheckBox* choice) const;
	void set_all(bool value);
	std::unordered_set<CheckBox*> boxes;
	CheckBox* choice;
};

namespace tz::ui
{
	bool moused_over(const GUI* gui, Vector2F mouse_position);
	bool left_clicked(const GUI* gui, const MouseListener& mouse_listener);
	bool right_clicked(const GUI* gui, const MouseListener& mouse_listener);
}

#endif