#ifndef GUI_HPP
#define GUI_HPP
#include <optional>
#include <functional>
#include <string>
#include <unordered_map>
#include <set>
#include <deque>
#include "graphics.hpp"
#include "../listener.hpp"
#include "../vector.hpp"
#include "mesh.hpp"
#include "shader.hpp"

class Window;
class Listener;

/**
 * Represents a Topaz GUI element.
 * Abstract. Not available for non-polymorphic use.
 * Inherit from this to create custom Topaz GUI yourself.
 *
 * Note on Topaz GUI
 * Topaz does not use any library for GUI; it completely reinvents the wheel.
 * If you have your own library for GUI, Topaz should stay out of your way, as long as you have your own windowing functionality.
 * An important note: You won't be able to use Engine to help you develop with Topaz unless you also use this GUI. If you are not using Topaz Windows but wish to use Engine anyway, consider writing your own subclass of Engine.
 */
class GUI
{
public:
	/**
	 * Construct a GUI element with all specifications.
	 * @param x - Position of the element on the x-axis.
	 * @param y - Position of the element on the y-axis.
	 * @param width - Width of the element, in pixels.
	 * @param height - Height of the element, in pixels.
	 * @param shader - Shader with which to render the GUI element.
	 */
	GUI(float x, float y, float width, float height, std::optional<std::reference_wrapper<Shader>> shader);

	/**
	 * Update this GUI element and all their children.
	 */
	virtual void update();
	/**
	 * Destroy all children, and then itself.
	 */
	virtual void destroy();
	/**
	 * Pure virtual.
	 * @return - N/A
	 */
	virtual bool focused() const = 0;
	/**
	 * Pure virtual
	 * @return - N/A
	 */
	virtual bool is_window() const = 0;
	/**
	 * Abstract GUI elements are mouse-insensitive.
	 * @return - False
	 */
	virtual bool is_mouse_sensitive() const{return false;}
	/**
	 * Get the position of the left-most of the GUI element, including parental offsets.
	 * @return - Left position of the element.
	 */
	virtual float get_window_pos_x() const;
	/**
	 * Get the position of the bottom-most of the GUI element, including parental offsets.
	 * @return - Bottom position of the element.
	 */
	virtual float get_window_pos_y() const;
	/**
	 * Read the x-coordinate of the GUI element.
	 * @return - X-coordinate of the element.
	 */
	float get_x() const;
	/**
	 * Read the y-coordinate of the GUI element.
	 * @return - Y-coordinate of the element.
	 */
	float get_y() const;
	/**
	 * Read the width of the GUI element, in pixels.
	 * @return - Width, in pixels.
	 */
	virtual float get_width() const;
	/**
	 * Read the height of the GUI element, in pixels.
	 * @return - Height, in pixels.
	 */
	virtual float get_height() const;
	/**
	 * Assign the x-coordinate of the GUI element.
	 * @param x - Desired x-coordinate.
	 */
	void set_x(float x);
	/**
	 * Assign the y-coordinate of the GUI element.
	 * @param y - Desired y-coordinate.
	 */
	void set_y(float y);
	/**
	 * Assign the width of the GUI element, in pixels.
	 * @param width - Desired width, in pixels.
	 */
	void set_width(float width);
	/**
	 * Assign the height of the GUI element, in pixels.
	 * @param height - Desired height, in pixels.
	 */
	void set_height(float height);
	/**
	 * Find the Window that contains this GUI element.
	 * @return - Pointer to the Window containing this element. If there is no such window, nullptr is returned
	 */
	Window* find_window_parent() const;
	/**
	 * Existential quantifier for this GUI element being contained within a Window.
	 * @return - True if a Window contains this element. False otherwise
	 */
	bool has_window_parent() const;
	/**
	 * Read the optional reference for the Shader used to render this element.
	 * @return - Read-only optional reference-wrapper for a Shader.
	 */
	const std::optional<std::reference_wrapper<Shader>>& get_shader() const;
	/**
	 * Queries whether there exists a Shader contained in this GUI element.
	 * @return
	 */
	bool has_shader() const;
	/**
	 * Query the Window for the GUI element which has this as a child.
	 * @return - Pointer to the GUI element if there is a Window parent and this parent exists. Otherwise, nullptr is returned
	 */
	GUI* get_parent() const;
	/**
	 * Prompt the Window to assign this to be a child of a specified GUI element
	 * @param parent - The GUI element who should have this element as a child.
	 */
	void set_parent(GUI* parent);
	/**
	 * Read the container of all of this GUI element's children.
	 * @return - Container of all children
	 */
	const std::deque<GUI*>& get_children() const;
	/**
	 * Add an existing GUI element as a child.
	 * @param child - The GUI element to become a child of this element.
	 */
	void add_child(GUI* child);
	/**
	 * Remove an existing GUI element child. Does not destroy the child, just de-registers it as a child of this element.
	 * @param child - The existing GUI element child to de-register.
	 */
	void remove_child(GUI* child);
	/**
	 * Query whether this GUI element is currently hidden.
	 * @return - True if the GUI element is hidden. False otherwise
	 */
	bool is_hidden() const;
	/**
	 * Specify whether this GUI element should be hidden.
	 * @param hidden - Desired hidden state.
	 */
	virtual void set_hidden(bool hidden);
	/**
	 * Specify whether this GUI element should use proportional positioning.
	 * Proportional positioning enabled means x, y, width and height should be clamped between 0.0-1.0 instead of in pixels.
	 * @param use_proportional_positioning - Desired proportional-positioning state.
	 */
	void set_using_proportional_positioning(bool use_proportional_positioning);
	/**
	 * Query whether this GUI element is set to be using proportional-positioning.
	 * @return - True if this GUI element uses proportional-positioning. False otherwise
	 */
	bool is_using_proportional_positioning() const;
	/**
	 * Find the GUI element that is responsible for covering this element.
	 * @return - The GUI element covering this element. If nothing is covering this, then nullptr is returned
	 */
	GUI* covered_by() const;
	/**
	 * Queries whether this GUI element is being covered by another GUI element in the Window.
	 * @return - True if there exists a GUI element covering this up. False otherwise
	 */
	bool covered() const;
protected:
	/// X-coordinate of the GUI element. In pixels, or between 0.0-1.0 if proportional-positioning is enabled.
	float x;
	/// Y-coordinate of the GUI element. In pixels, or between 0.0-1.0 if proportional-positioning is enabled.
	float y;
	/// Width of the GUI element. In pixels, or between 0.0-1.0 if proportional-positioning is enabled.
	float width;
	/// Width of the GUI element. In pixels, or between 0.0-1.0 if proportional-positioning is enabled.
	float height;
	/// Reference to a Shader, which may or may not exist.
	std::optional<std::reference_wrapper<Shader>> shader;
	/// Pointer to the GUI element which has this registered as a child.
	GUI* parent;
	/// Container of all children.
	std::deque<GUI*> children;
	/// Stores whether this GUI element is currently hidden.
	bool hidden;
	/// Stores whether this GUI element is currently set to be using proportional-positioning.
	bool use_proportional_positioning;
};

/**
* Topaz Windows used to draw on the screen.
* Topaz's graphics module will not initialise fully until at least one instance of this class is instantiated.
*/
class Window : public GUI
{
public:
	/**
	 * Construct a Window with specifications.
	 * @param w - Width of the Window, in pixels.
	 * @param h - Height of the Window, in pixels.
	 * @param title - String representing the desired title of the Window.
	 */
	Window(int w = 800, int h = 600, std::string title = "Untitled");
	/**
	 * Construct a Window from an existing Window.
	 * @param copy - The Window to be copied.
	 */
	Window(const Window& copy);
	/**
	 * Windows are not movable, as nothing meaningful can be done with an existing OpenGL-Context.
	 * @param move - N/A
	 */
	Window(Window&& move) = delete;
	/**
	 * Dispose of the OpenGL-Context as its guaranteed to no longer be used.
	 */
	virtual ~Window();
	/**
	 * Windows cannot be re-assigned. Use reconstruction if necessary.
	 * @param rhs - N/A
	 * @return - N/A
	 */
	Window& operator=(const Window& rhs) = delete;

	/**
	 * Update (and render) the window and all the GUI elements it contains.
	 */
	virtual void update() override;
	/**
	 * Dispose of the Window and all of the GUI elements contained within it.
	 */
	virtual void destroy() override;
	/**
	 * Query whether the window is focused.
	 * @return - True if the Window is focused. Otherwise false
	 */
	virtual bool focused() const override;
	/**
	 * Window is classified formally as a Window.
	 * @return - True
	 */
	virtual bool is_window() const override{return true;}
	/**
	 * Unused for Window, used only in subclasses.
	 * @return - N/A
	 */
	virtual float get_window_pos_x() const override{return 0.0f;}
	/**
	 * Unused for Window, used only in subclasses.
	 * @return - N/A
	 */
	virtual float get_window_pos_y() const override{return 0.0f;}
	/**
	 * Specify whether the Window should be hidden or not.
	 * @return - State of the Window being hidden.
	 */
	virtual void set_hidden(bool hidden) override;
	/**
	 * Represents which technique for Window buffer-swapping can be used.
	 */
	enum class SwapIntervalType : int
	{
		LATE_SWAP_TEARING = -1,
		IMMEDIATE_UPDATES = 0,
		VSYNC = 1,
	};
	/**
	 * Query whether the user has requested that the Window be destroyed.
	 * @return - True if the user has requested close. False otherwise
	 */
	bool is_close_requested() const;
	/**
	 * Query which SwapIntervalType is being used for this Window.
	 * @return - The SwapIntervalType this Window is using
	 */
	SwapIntervalType get_swap_interval_type() const;
	/**
	 * Specify which SwapIntervalType should be used for this Window.
	 * @param type - The SwapIntervalType the Window should use
	 */
	void set_swap_interval_type(SwapIntervalType type) const;
	/**
	 * Change the string shown on the Window title.
	 * @param new_title - The new title of the Window.
	 */
	void set_title(const std::string& new_title);
	/**
	 * Represents which type of Fullscreen states are available for a Window.
	 */
	enum class FullscreenType : Uint32
	{
		VIDEO_MODE = SDL_WINDOW_FULLSCREEN,
		DESKTOP_MODE = SDL_WINDOW_FULLSCREEN_DESKTOP,
		WINDOWED_MODE = 0,
	};
	/**
	 * Query whether the Window is in either of the two fullscreen-types.
	 * @return - True if the Window is in either of the two fullscreen-types. False otherwise
	 */
	bool is_fullscreen() const;
	/**
	 * Get the fullscreen-type that the Window is currently in.
	 * @return - Either VIDEO_MODE (true fullscreen), DESKTOP_MODE (fake fullscreen) or WINDOWED_MODE (not fullscreen).
	 */
	FullscreenType get_fullscreen() const;
	/**
	 * Specify the fullscreen-type that the Window should be in.
	 * @param type - FullscreenType the Window should be in.
	 */
	void set_fullscreen(FullscreenType type) const;
	/**
	 * Ensure that all OpenGL render calls in the future render to this Window's framebuffer.
	 */
	void set_render_target() const;
	/**
	 * Ensures that none of the Window's GUI elements have focus.
	 */
	void clear_focus();
	/**
	 * Clear the Window's framebuffer and set the background to a desired colour.
	 * @param mask - Which buffer-bits to clear
	 * @param r - Redness of the background colour, between 0.0 and 1.0
	 * @param g - Greenness of the background colour, between 0.0 and 1.0
	 * @param b - Blueness of the background colour, between 0.0 and 1.0
	 * @param a - Alpha intensity of the background colour, between 0.0 and 1.0
	 */
	void clear(GLbitfield mask = (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT), float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f) const;
	/**
	 * Register an existing listener to this Window.
	 * @param l - The polymorphic listener that this Window should administrate
	 */
	void register_listener(Listener& l);
	/**
	 * Deregister a listener that is currently being administrated by this Window.
	 * @param l - The listener which should be de-registered
	 */
	void deregister_listener(Listener& l);
	/**
	 * Get the GUI element child which is in focus.
	 * @return - Pointer to the GUI element which is focused. If none are focused, nullptr is returned
	 */
	GUI* get_focused_child() const;
	/**
	 * Specify the GUI element child which should be in focus.
	 * @param child - Pointer to the GUI element which should be focused. If nullptr is passed, no GUI element will be focused.
	 */
	void set_focused_child(GUI* child);
private:
	using GUI::get_x;
	using GUI::get_y;
	using GUI::set_x;
	using GUI::set_y;
	/// Container of all registered polymorphic listeners.
	std::unordered_map<unsigned int, Listener*> registered_listeners;
	/// String to be displayed as the Window title.
	std::string title;
	/// Stores whether the user has requested this Window to close.
	bool close_requested;
	/// SDL_Window underlying handle.
	SDL_Window* sdl_window_pointer;
	/// The SDL_Window's OpenGL context handle.
	SDL_GLContext sdl_gl_context_handle;
	/// The GUI element child which is focused.
	GUI* focused_child;
};

namespace tz::ui
{
	/**
	* Performs a depth-first-traversal (DFT) through the 'gui' child tree to return a set of all children, and all their children's children etc...
	*/
	std::set<GUI*> descendants(const GUI* gui, bool visible_only = false);
	/**
	* Performs a DFT similarly to tz::ui::descendants, but only the children whom do not have any children themselves i.e the 'youngest' descendants.
	*/
	std::set<GUI*> youngest_descendants(const GUI* gui);
	/**
	* Create an orthographic row-major projection matrix using the gui window parent's dimensions.
	* If the parameter does not have a window parent, returns an identity matrix.
	*/
	Matrix4x4 create_orthographic_gui_matrix(const GUI* gui);
}
#endif