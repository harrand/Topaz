#ifndef GUI_WIDGET_HPP
#define GUI_WIDGET_HPP
#include "gui_display.hpp"

/**
* Just like a TextLabel, but also is mouse-sensitive and pressable to execute a command.
*/
class Button : public TextLabel
{
public:
	/**
	 * Construct a Button with all required specifications.
	 * @param x - Position of the button on the x-axis, in pixels.
	 * @param y - Position of the button on the y-axis, in pixels.
	 * @param colour - RGBA-coded colour of the button text
	 * @param background_colour - RGBA-coded colour of the button background
	 * @param text_border_colour - Optional RGBA-coded colour of the button text border
	 * @param font - Font used to render the text.
	 * @param text - String representing the text displayed on the button body.
	 * @param shader - The shader with which to render the button.
	 * @param mouse_listener - The mouse-listener responsible for providing button-press and mouse-over events
	 */
	Button(float x, float y, Vector4F colour, std::optional<Vector4F> background_colour, std::optional<Vector3F> text_border_colour, Font font, const std::string& text, Shader& shader, MouseListener& mouse_listener);
	/**
	 * Render the button and check for mouse-overs and button-presses. Also updates all children.
	 */
	virtual void update() override;
	/**
	 * Query whether the button has focus or not.
	 * @return - True if the button has focus. False otherwise.
	 */
	virtual bool focused() const override;
	/**
	 * Buttons are mouse sensitive.
	 * @return - True
	 */
	virtual bool is_mouse_sensitive() const override{return true;}
	/**
	 * Read-only access to the command executed when the Button is moused-over.
	 * @return - Polymorphic command which should be executed when the button is moused-over.
	 */
	Command* get_on_mouse_over() const;
	/**
	 * Read-only access to the command executed when the Button is pressed.
	 * @return - Polymorphic command which should be executed when the Button is pressed.
	 */
	Command* get_on_mouse_click() const;
	/**
	 * Specify a new command to be executed when the button is moused-over.
	 * @param cmd - Polymorphic command to be invoked if the button is moused-over.
	 */
	void set_on_mouse_over(Command* cmd);
	/**
	 * Specify a new command to be executed when the button is clicked.
	 * @param cmd - Polymorphic command to be invoked if the button is clicked.
	 */
	void set_on_mouse_click(Command* cmd);
	/**
	 * Query whether the button is currently being moused-over or not
	 * @return - True if the button is moused-over. False otherwise.
	 */
	bool moused_over() const;
	/**
	 * Query whether the button is currently being clicked on or not
	 * @return - True if the button is being clicked on. False otherwise.
	 */
	bool clicked_on() const;
protected:
	/// MouseListener to control mouse events.
	MouseListener& mouse_listener;
	/// Stores whether the Button was clicked last frame.
	bool just_clicked;
	/// Stores whether the Button was moused-over last frame.
	bool just_moused_over;
private:
	/// The Command to be executed when the Button is moused-over. If nullptr, nothing happens.
	Command* on_mouse_over;
	/// The Command to be executed when the Buttin is clicked. If nullptr, nothing happens.
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
	/**
	 * Construct a CheckBox with all specifications.
	 * @param x - Position of the checkbox on the x-axis, in pixels.
	 * @param y - Position of the checkbox on the y-axis, in pixels.
	 * @param width - Width of the checkbox, in pixels.
	 * @param height - Height of the checkbox, in pixels.
	 * @param colour_on - Colour of the checkbox when it is checked (on).
	 * @param colour_off - Colour of the checkbox when it is unchecked (off).
	 * @param shader - The shader with which to render the checkbox.
	 * @param mouse_listener - The mouse-listener with which to query whether is being clicked or not.
	 * @param ticked - Whether the checkbox should initially be checked or not.
	 */
	Checkbox(float x, float y, float width, float height, Vector4F colour_on, Vector4F colour_off, Shader& shader, MouseListener& mouse_listener, bool ticked = false);
	/**
	 * Render and update all children.
	 */
	virtual void update() override;
	/**
	 * Query whether the checkbox is focused or not.
	 * @return - True if the checkbox is focused. False otherwise
	 */
	virtual bool focused() const override;
	/**
	 * Checkboxes are always mouse sensitive.
	 * @return - True
	 */
	virtual bool is_mouse_sensitive() const override{return true;}
	/**
	 * Query whether the checkbox is currently being moused-over.
	 * @return - True if the checkbox is moused-over. False otherwise
	 */
	bool moused_over() const;
	/**
	 * Query whether the checkbox is currently being clicked on.
	 * @return - True if the checkbox is being clicked on. False otherwise
	 */
	bool clicked_on() const;
	/**
	 * Get RGBA-coded colour of the checkbox when it is checked.
	 * @return - Colour of the checkbox when checked.
	 */
	const Vector4F& get_colour_on() const;
	/**
	 * Get RGBA-coded colour of the checkbox when it is unchecked.
	 * @return - Colour of the checkbox when unchecked.
	 */
	const Vector4F& get_colour_off() const;
	/**
	 * Get RGBA-coded colour of the checkbox. This colour depends on whether it is checked or not.
	 * @return - Current colour of the checkbox.
	 */
	Vector4F get_colour() const;
	
	friend class CheckboxChoice;
	/// Stores whether the Checkbox is checked or not.
	bool value;
protected:
	/**
	 * Queries the CheckboxChoice parent whether this is the chosen Checkbox.
	 * @return - True if there exists a CheckboxChoice parent, and it specifies this Checkbox as the selected box. False otherwise
	 */
	bool is_choice() const;

	/// The colour of the Checkbox foreground when the box is checked.
	Vector4F colour_on;
	/// The colour of the Checkbox foreground when the box is unchecked.
	Vector4F colour_off;
	/// MouseListener to control mouse events.
	MouseListener& mouse_listener;
	/// Stores whether the Button was clicked last frame.
	bool just_clicked;
	/// Stores whether the Button was moused-over last frame.
	bool just_moused_over;
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
	/**
	 * Construct a Slider with all specifications.
	 * @param x - Position of the slider on the x-axis, in pixels
	 * @param y - Position of the slider on the y-axis, in pixels
	 * @param width - Width of the slider, in pixels
	 * @param height - Height of the slider, in pixels
	 * @param slider_colour - Colour of the slider's wedge.
	 * @param background_colour - Background colour of the slider body.
	 * @param slider_size - 2-dimensional Vector representing the size of the slider's wedge.
	 * @param shader - The shader with which to render the slider.
	 * @param mouse_listener - The mouse-listener with which to query whether the slider needs to have its wedge dragged
	 * @param position - Current position of the slider wedge, clamped between 0.0 and 1.0
	 */
	Slider(float x, float y, float width, float height, Vector4F slider_colour, Vector4F background_colour, Vector2F slider_size, Shader& shader, MouseListener& mouse_listener, float position = 0.0f);
	/**
	 * Render and update all children.
	 */
	virtual void update() override;
	/**
	 * Query whether the slider is currently focused.
	 * @return - True if the slider is focused. False otherwise
	 */
	virtual bool focused() const override;
	/**
	 * Sliders are mouse-sensitive.
	 * @return - True
	 */
	virtual bool is_mouse_sensitive() const override{return true;}
	/**
	 * Query whether the slider is currently being moused-over.
	 * @return - True if the slider is moused-over. False otherwise
	 */
	bool moused_over() const;
	/**
	 * Query whether the slider is currently being clicked on.
	 * @return - True if the slider is being clicked on. False otherwise
	 */
	bool clicked_on() const;
	/**
	 * Get the RGBA-coded colour of the slider wedge.
	 * @return - Colour of the slider wegde.
	 */
	const Vector4F& get_slider_colour() const;
	/**
	 * Get the dimensions of the slider.
	 * @return - 2-dimensional Vector representing the size of the wedge of the slider.
	 */
	const Vector2F& get_slider_size() const;

	/// Position of the slider wedge on the slider. Clamped between 0.0 and 1.0.
	double position;
private:
	/// Colour of the slider wedge.
	Vector4F slider_colour;
	/// 2-dimensional Vector representing the size of the slider wedge.
	Vector2F slider_size;
	/// MouseListener to control mouse events.
	MouseListener& mouse_listener;
	/// Stores whether the Button was clicked last frame.
	bool just_clicked;
	/// Stores whether the Button was moused-over last frame.
	bool just_moused_over;
};

/**
* Non-owning helper class to manage multiple Checkboxes.
* Use this to allow multiple checkboxes to only have one truthy at a time.
*/
class CheckboxChoice
{
public:
	/**
	 * Construct a CheckboxChoice from existing CheckBoxes. Does not own any of the CheckBoxes.
	 * @param boxes - List of all the CheckBoxes in the choice.
	 * @param initial_choice - Initial CheckBox in the list that should be checked.
	 */
	CheckboxChoice(std::initializer_list<Checkbox*> boxes, Checkbox* initial_choice = nullptr);
	/**
	 * Construct a CheckboxChoice from existing CheckBoxes. Does not own any of the CheckBoxes.
	 * @param boxes - List of all the CheckBoxes in the choice.
	 * @param initial_choice - Initial CheckBox in the list that should be checked.
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
	/**
	* Copy assignment operator is also deleted for the same reason; choice-fighting.
	*/
	CheckboxChoice& operator=(const CheckboxChoice& rhs) = delete;
	/**
	* Similarly to the move constructor, the move assignment-operator cannot induce choice-fighting so remains available for use.
	*/
	CheckboxChoice& operator=(CheckboxChoice&& rhs) = default;
	/**
	 * Queries whether there is a discernible choice made.
	 * @return - True if there is a choice. False otherwise
	 */
	bool has_choice() const;
	/**
	 * Get the Checkbox which is currently checked in the choice.
	 * @return - Pointer to the checkbox which is chosen.
	 */
	Checkbox* get_choice() const;
	/**
	 * Check the selected choice.
	 * @param choice - The checkbox to be checked. This will uncheck all the other checkboxes in the choice
	 */
	void set_choice(Checkbox* choice);
	/**
	 * Get all the checkboxes in the choice.
	 * @return - All the checkboxes
	 */
	const std::unordered_set<Checkbox*>& get_bool_boxes() const;
private:
	/**
	 * Queries whether a Checkbox is part of a CheckboxChoice.
	 * @param choice - The Checkbox to query whether belongs to this choice or not
	 * @return - True if the Checkbox belongs to this choice. False otherwise
	 */
	bool choice_in_scope(Checkbox* choice) const;
	/**
	 * Sets all the Checkboxes in the choice to a certain check-state.
	 * @param value - The state to set all checkboxes.
	 */
	void set_all(bool value);
	/// Container of all the Checkboxes in this choice.
	std::unordered_set<Checkbox*> boxes;
	/// Pointer to the Checkbox which is checked.
	Checkbox* choice;
};

namespace tz::ui
{
	bool moused_over(const GUI* gui, Vector2F mouse_position);
	bool left_clicked(const GUI* gui, const MouseListener& mouse_listener);
	bool right_clicked(const GUI* gui, const MouseListener& mouse_listener);
}

#endif