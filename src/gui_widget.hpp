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

class BoolBoxChoice;

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
	
	friend class BoolBoxChoice;
	
	bool value;
protected:
	bool is_choice() const;

	Vector4F colour_on, colour_off;
	MouseListener& mouse_listener;
	bool just_clicked, just_moused_over;
	/**
	* choice_parent is handled purely by the friend-class BoolBoxChoice.
	*/
	BoolBoxChoice* choice_parent;
};

/**
* Non-owning helper class to manage multiple BoolBoxes.
*/
class BoolBoxChoice
{
public:
	/**
	* Non-owning pointer initialisation.
	*/
	BoolBoxChoice(std::initializer_list<BoolBox*> boxes, BoolBox* initial_choice = nullptr);
	/**
	* Non-owning reference initialisation.
	*/
	BoolBoxChoice(std::initializer_list<std::reference_wrapper<BoolBox>> boxes, BoolBox* initial_choice = nullptr);
	/**
	* Copy constructor deleted.
	* This is because any two BoolBoxChoices may not share a single element, or "choice-fighting" will occur.
	* Choice-fighting is the phenomenon such that multiple BoolBoxChoices choose different boxes to truthify, causing the latter to always invalidate the former.
	*/
	BoolBoxChoice(const BoolBoxChoice& copy) = delete;
	/**
	* Move constructor implies the rvalue-reference parameter is about to go out-of-scope, meaning that it will not be available to cause choice-fighting.
	* For this reason, the move constructor is available.
	*/
	BoolBoxChoice(BoolBoxChoice&& move) = default;
	~BoolBoxChoice() = default;
	/**
	* Copy assignment operator is also deleted for the same reason; choice-fighting.
	*/
	BoolBoxChoice& operator=(const BoolBoxChoice& rhs) = delete;
	/**
	* Similarly to the move constructor, the move assignment-operator cannot induce choice-fighting so remains available for use.
	*/
	BoolBoxChoice& operator=(BoolBoxChoice&& rhs) = default;
	
	bool has_choice() const;
	BoolBox* get_choice() const;
	void set_choice(BoolBox* choice);
	const std::unordered_set<BoolBox*>& get_bool_boxes() const;
private:
	bool choice_in_scope(BoolBox* choice) const;
	void set_all(bool value);
	std::unordered_set<BoolBox*> boxes;
	BoolBox* choice;
};

namespace tz::ui
{
	bool moused_over(const GUI* gui, Vector2F mouse_position);
	bool left_clicked(const GUI* gui, const MouseListener& mouse_listener);
	bool right_clicked(const GUI* gui, const MouseListener& mouse_listener);
}

#endif