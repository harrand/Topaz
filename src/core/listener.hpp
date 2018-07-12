#ifndef LISTENER_HPP
#define LISTENER_HPP

#include "core/window.hpp"
#include "data/vector.hpp"
#include <SDL2/SDL.h>

class Window;
/**
* Wrapper for an SDL_Event listener.
* Abstract. Not available for non-polymorphic use.
* Inherit from this to create custom listeners that a Topaz Window can register.
*/
class Listener
{
public:
	/// Trivial construction.
	Listener();

	friend class Window;
	friend class GUIWidget;

	/**
	 * Pure virtual.
	 * @param evt - N/A
	 */
	virtual void handle_events(const SDL_Event& evt) = 0;
protected:
	Window* window;
};

/**
* How Topaz handles mouse input.
* Register this to a Topaz Window to use properly.
*/
class MouseListener: public Listener
{
public:
	/// Fall-through constructor
	MouseListener();
	/**
	 * Construct the Listener and instantly attach it to an existing Window.
	 * @param window - The Window to attach this Listener to
	 */
	MouseListener(Window& window);

	/**
	 * Invoked when the parent Window polls all pending events.
	 * @param evt - The SDL Event object passed from the Window event-poll
	 */
	virtual void handle_events(const SDL_Event& evt) override;
	/**
	 * Clear previously-stored mouse-position information.
	 */
	void reload_mouse_delta();
	/**
	 * Query whether the mouse's left-click button is currently pressed.
	 * @return - True if the mouse is left-clicking. False otherwise
	 */
	bool is_left_clicked() const;
	/**
	 * Query whether the mouse's right-click button is currently pressed.
	 * @return - True if the mouse is right-clicking. False otherwise
	 */
	bool is_right_clicked() const;
	/**
	 * Get a 2-dimensional Vector representing the mouse-position on the screen, in pixels.
	 * @return - Mouse-position in pixels
	 */
	const Vector2F& get_mouse_position() const;
	/**
	 * Get a 2-dimensional Vector representing the change in mouse-position on the screen since the last event-poll, in pixels
	 * @return - Mouse-delta in pixels
	 */
	Vector2F get_mouse_delta_position() const;
	/**
	 * Get a 2-dimensional Vector representing the position on the screen that the most recent left-click took place, in pixels.
	 * @return - Last left-click position on the screen
	 */
	const Vector2F& get_left_click_location() const;
	/**
	 * Get a 2-dimensional Vector representing the position on the screen that the most recent right-click took place, in pixels.
	 * @return - Last right-click position on the screen
	 */
	const Vector2F& get_right_click_location() const;
private:
	/// Stores which mouse-buttons are being pressed currently.
	bool left_click, right_click;
	/// Stores the most recent mouse-click locations.
	Vector2F left_click_location, right_click_location;
	/// Stores the most recent mouse location, in addition to the second-most recent mouse location.
	Vector2F previous_mouse_position, mouse_position;
};

/**
* How Topaz handles keyboard input.
* Register this to a Topaz Window to use properly.
*/
class KeyListener: public Listener
{
public:
	/// Fall-through constructor.
	KeyListener();
	/**
	 * Construct the Listener and instantly attach it to an existing Window.
	 * @param window - The Window to attach this Listener to
	 */
	KeyListener(Window& window);

	/**
	 * Invoked when the parent Window polls all pending events.
	 * @param evt - The SDL Event object passed from the Window event-poll
	 */
	virtual void handle_events(const SDL_Event& evt) override;
	/**
	 * Query whether the keyboard-key with the specified name is currently pressed.
	 * @param keyname - Name of the specified key, such as "K" or "Enter"
	 * @return - True if the key is pressed. Otherwise false
	 */
	bool is_key_pressed(const std::string& keyname) const;
	/**
	 * Query whether the keyboard-key with the specified name is currently being released.
	 * @param keyname - Name of the specified key, such as "K" or "Enter"
	 * @return - True if the key is being released. Otherwise false
	 */
	bool is_key_released(const std::string& keyname) const;
	/**
	 * Query whether the keyboard-key with the specified name is currently pressed.
	 * This method will make itself return false next invocation if the same key is pressed.
	 * @param keyname - Name of the specified key, such as "K" or "Enter"
	 * @return - True if the key is pressed. Otherwise false
	 */
	bool catch_key_pressed(const std::string& keyname);
	/**
	 * Query whether the keyboard-key with the specified name is currently being released.
	 * This method will make itself return false next invocation if the same key is pressed.
	 * @param keyname - Name of the specified key, such as "K" or "Enter"
	 * @return - True if the key is being released. Otherwise false
	 */
	bool catch_key_released(const std::string& keyname);
    virtual void on_key_pressed([[maybe_unused]] const std::string& keyname){}
    virtual void on_key_released([[maybe_unused]] const std::string& keyname){}
private:
	/// Container of all the keys being pressed.
	std::vector<std::string> pressed_keys;
	/// Container of all the keys being released.
	std::vector<std::string> released_keys;
};

namespace tz::listener
{
	/**
	 * Query whether this Listener is a MouseListener or not.
	 * @param listener - The Listener to query
	 * @return - True if the Listener is a MouseListener. Otherwise false
	 */
	bool is_mouse(const Listener* listener);
	/**
	 * Query whether this Listener is a KeyListener or not.
	 * @param listener - The Listener to query
	 * @return - True if the Listener is a KeyListener. Otherwise false
	 */
	bool is_keyboard(const Listener* listener);
}

#endif