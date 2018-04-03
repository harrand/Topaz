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
	Button(const Button& rhs) = default;
	Button(Button&& move) = default;
	~Button() = default;
	Button& operator=(const Button& rhs) = default;

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
	Checkbox(const Checkbox& copy) = default;
	Checkbox(Checkbox&& move) = default;
	~Checkbox() = default;
	Checkbox& operator=(const Checkbox& rhs) = default;

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
	Slider(const Slider& copy) = default;
	Slider(Slider&& move) = default;
	~Slider() = default;
	Slider& operator=(const Slider& rhs) = default;

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
	~CheckboxChoice() = default;
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