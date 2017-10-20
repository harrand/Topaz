#ifndef GUI_HPP
#define GUI_HPP
#include <optional>
#include <functional>
#include <string>
#include <unordered_map>
#include <set>
#include <deque>
#include "graphics.hpp"
#include "listener.hpp"
#include "vector.hpp"
#include "mesh.hpp"

class Window;

class GUI
{
public:
	GUI(float x, float y, float width, float height, std::optional<std::reference_wrapper<const Shader>> shader);
	GUI(const GUI& copy) = default;
	GUI(GUI&& move) = default;
	virtual ~GUI() = default;
	GUI& operator=(const GUI& rhs) = default;
	
	virtual void update();
	virtual void destroy();
	virtual bool focused() const = 0;
	virtual bool is_window() const = 0;
	virtual bool is_mouse_sensitive() const{return false;}
	virtual float get_window_pos_x() const;
	virtual float get_window_pos_y() const;
	float get_x() const; float get_y() const; float get_width() const; float get_height() const;
	void set_x(float x); void set_y(float y); void set_width(float width); void set_height(float height);
	Window* find_window_parent() const;
	bool has_window_parent() const;
	const std::optional<std::reference_wrapper<const Shader>> get_shader() const;
	bool has_shader() const;
	GUI* get_parent() const;
	void set_parent(GUI* parent);
	const std::deque<GUI*>& get_children() const;
	void add_child(GUI* child);
	void remove_child(GUI* child);
	bool is_hidden() const;
	void set_hidden(bool hidden);
	void set_using_proportional_positioning(bool use_proportional_positioning);
	bool is_using_proportional_positioning() const;
	bool covered() const;
protected:
	float x, y, width, height;
	std::optional<std::reference_wrapper<const Shader>> shader;
	GUI* parent;
	std::deque<GUI*> children;
	bool hidden, use_proportional_positioning;
};

class Window : public GUI
{
public:
	Window(int w = 800, int h = 600, std::string title = "Untitled");
	Window(const Window& copy);
	Window(Window&& move) = delete;
	virtual ~Window();
	Window& operator=(const Window& rhs) = delete;
	
	virtual void update() override;
	virtual void destroy() override;
	virtual bool focused() const override;
	virtual bool is_window() const override;
	virtual float get_window_pos_x() const override{return 0.0f;}
	virtual float get_window_pos_y() const override{return 0.0f;}
	enum class SwapIntervalType : int
	{
		LATE_SWAP_TEARING = -1,
		IMMEDIATE_UPDATES = 0,
		VSYNC = 1,
	};
	bool is_close_requested() const;
	void set_swap_interval_type(SwapIntervalType type) const;
	SwapIntervalType get_swap_interval_type() const;
	void set_title(const std::string& new_title);
	void set_render_target() const;
	void clear_focus();
	SDL_Window* get_window_handle() const;
	void clear(float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f) const;
	void register_listener(Listener& l);
	void deregister_listener(Listener& l);
	GUI* get_focused_child() const;
	void set_focused_child(GUI* child);
private:
	using GUI::get_x;
	using GUI::get_y;
	using GUI::set_x;
	using GUI::set_y;
	std::unordered_map<unsigned int, Listener*> registered_listeners;
	std::string title;
	bool close_requested;
	SDL_Window* sdl_window_pointer;
	SDL_GLContext sdl_gl_context_handle;
	GUI* focused_child;
};

namespace tz::ui
{
	// O(n log n) even through depth-first-search is normally O(n) but set insert is O(log n) so yeah
	std::set<GUI*> descendants(const GUI* gui, bool visible_only = false);
	// still O(n log n) (assuming range based for loop optimises to not recompute every tz::ui::descendants() (otherwise would go to O(n log^2 n) which is horrific))
	std::set<GUI*> youngest_descendants(const GUI* gui);
	Matrix4x4 create_orthographic_gui_matrix(const GUI* gui);
}
#endif