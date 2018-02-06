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
	/**
	* Read-only access to the command executed when the Button is moused-over.
	*/
	Command* get_on_mouse_over() const;
	/**
	* Read-only access to the command executed when the Button is pressed.
	*/
	Command* get_on_mouse_click() const;
	/**
	* Change what happens when the button is moused-over.
	* Inputting 'nullptr' will mean that nothing happens.
	*/
	void set_on_mouse_over(Command* cmd);
	/**
	* Change what happens when the button is left-clicked.
	* Inputting 'nullptr' will mean that nothing happens.
	*/
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

class CheckboxChoice;

/**
* Graphical representation of a mutable boolean.
* Use this to enable user-input for toggling booleans.
*/
class Checkbox : public Panel
{
public:
	Checkbox(float x, float y, float width, float height, Vector4F colour_on, Vector4F colour_off, Shader& shader, MouseListener& mouse_listener, bool ticked = false);
	Checkbox(const Checkbox& copy) = default;
	Checkbox(Checkbox&& move) = default;
	~Checkbox() = default;
	Checkbox& operator=(const Checkbox& rhs) = default;
	
	virtual void update() override;
	virtual bool focused() const override;
	virtual bool is_mouse_sensitive() const override{return true;}
	bool moused_over() const;
	bool clicked_on() const;
	
	/**
	* Get colour of the checkbox when the value is true.
	*/
	const Vector4F& get_colour_on() const;
	/**
	* Get colour of the checkbox when the value is false.
	*/
	const Vector4F& get_colour_off() const;
	/**
	* Retrieve the colour of the checkbox.
	* The colour depends on whether the value is true or false.
	*/
	Vector4F get_colour() const;
	
	friend class CheckboxChoice;
	
	bool value;
protected:
	bool is_choice() const;

	Vector4F colour_on, colour_off;
	MouseListener& mouse_listener;
	bool just_clicked, just_moused_over;
	/**
	* choice_parent is handled purely by the friend-class CheckboxChoice.
	*/
	CheckboxChoice* choice_parent;
};

/**
* Graphical representation of a mutable double.
* Use this to enable user-input for editing continuous data.
*/
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
	
	const Vector4F& get_slider_colour() const;
	const Vector2F& get_slider_size() const;
	
	double position;
private:
	Vector4F slider_colour;
	Vector2F slider_size;
	MouseListener& mouse_listener;
	bool just_clicked, just_moused_over;
};

/**
* Non-owning helper class to manage multiple Checkboxes.
* Use this to allow multiple checkboxes to only have one truthy at a time.
*/
class CheckboxChoice
{
public:
	/**
	* Non-owning pointer initialisation.
	*/
	CheckboxChoice(std::initializer_list<Checkbox*> boxes, Checkbox* initial_choice = nullptr);
	/**
	* Non-owning reference initialisation.
	*/
	CheckboxChoice(std::initializer_list<std::reference_wrapper<Checkbox>> boxes, Checkbox* initial_choice = nullptr);
	/**
	* Copy constructor deleted.
	* This is because any two CheckboxChoices may not share a single element, or "choice-fighting" will occur.
	* Choice-fighting is the phenomenon such that multiple CheckboxChoices choose different boxes to truthify, causing the latter to always invalidate the former.
	*/
	CheckboxChoice(const CheckboxChoice& copy) = delete;
	/**
	* Move constructor implies the rvalue-reference parameter is about to go out-of-scope, meaning that it will not be available to cause choice-fighting.
	* For this reason, the move constructor is available.
	*/
	CheckboxChoice(CheckboxChoice&& move) = default;
	~CheckboxChoice() = default;
	/**
	* Copy assignment operator is also deleted for the same reason; choice-fighting.
	*/
	CheckboxChoice& operator=(const CheckboxChoice& rhs) = delete;
	/**
	* Similarly to the move constructor, the move assignment-operator cannot induce choice-fighting so remains available for use.
	*/
	CheckboxChoice& operator=(CheckboxChoice&& rhs) = default;
	
	bool has_choice() const;
	Checkbox* get_choice() const;
	void set_choice(Checkbox* choice);
	const std::unordered_set<Checkbox*>& get_bool_boxes() const;
private:
	bool choice_in_scope(Checkbox* choice) const;
	void set_all(bool value);
	std::unordered_set<Checkbox*> boxes;
	Checkbox* choice;
};

namespace tz::ui
{
	bool moused_over(const GUI* gui, Vector2F mouse_position);
	bool left_clicked(const GUI* gui, const MouseListener& mouse_listener);
	bool right_clicked(const GUI* gui, const MouseListener& mouse_listener);
}

#endif