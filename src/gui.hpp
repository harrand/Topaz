#ifndef GUI_HPP
#define GUI_HPP
#include <optional>
#include <functional>
#include <string>
#include <unordered_map>
#include "graphics.hpp"
#include "listener.hpp"
#include "vector.hpp"
#include "mesh.hpp"

class Window;

class GUIElement
{
public:
	GUIElement(std::optional<std::reference_wrapper<const Shader>> shader);
	GUIElement(const GUIElement& copy) = default;
	GUIElement(GUIElement&& move) = default;
	virtual ~GUIElement() = default;
	GUIElement& operator=(const GUIElement& rhs) = default;
	
	virtual void update();
	virtual void destroy();
	virtual bool focused() const = 0;
	virtual bool is_window() const = 0;
	virtual bool is_mouse_sensitive() const = 0;
	virtual float get_window_pos_x() const = 0;
	virtual float get_window_pos_y() const = 0;
	Window* find_window_parent() const;
	bool has_window_parent() const;
	const std::optional<std::reference_wrapper<const Shader>> get_shader() const;
	bool has_shader() const;
	GUIElement* get_parent() const;
	void set_parent(GUIElement* parent);
	const std::unordered_set<GUIElement*>& get_children() const;
	void add_child(GUIElement* child);
	void remove_child(GUIElement* child);
	bool is_hidden() const;
	void set_hidden(bool hidden);
protected:
	std::optional<std::reference_wrapper<const Shader>> shader;
	GUIElement* parent;
	std::unordered_set<GUIElement*> children;
	bool hidden;
};

class Window : public GUIElement
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
	virtual bool is_mouse_sensitive() const override;
	virtual float get_window_pos_x() const override;
	virtual float get_window_pos_y() const override;
	enum class SwapIntervalType : int
	{
		LATE_SWAP_TEARING = -1,
		IMMEDIATE_UPDATES = 0,
		VSYNC = 1,
	};
	int get_width() const;
	int get_height() const;
	void set_width(float width);
	void set_height(float height);
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
	GUIElement* get_focused_child() const;
	void set_focused_child(GUIElement* child);
private:	
	std::unordered_map<unsigned int, Listener*> registered_listeners;
	int w, h;
	std::string title;
	bool close_requested;
	SDL_Window* sdl_window_pointer;
	SDL_GLContext sdl_gl_context_handle;
	GUIElement* focused_child;
};

class Panel : public GUIElement
{
public:
	Panel(float x, float y, float width, float height, Vector4F colour, const Shader& shader);
	
	float get_x() const;
	float get_y() const;
	virtual float get_window_pos_x() const override;
	virtual float get_window_pos_y() const override;
	float get_width() const;
	float get_height() const;
	const Vector4F& get_colour() const;
	void set_x(float x);
	void set_y(float y);
	void set_width(float width);
	void set_height(float height);
	void set_colour(Vector4F colour);
	virtual void update() override;
	virtual void destroy() override;
	virtual bool focused() const override;
	virtual bool is_window() const override;
	virtual bool is_mouse_sensitive() const override;
	void set_using_proportional_positioning(bool use_proportional_positioning);
	bool is_using_proportional_positioning() const;
protected:
	bool use_proportional_positioning;
	float x, y, width, height;
	Vector4F colour;
	Mesh quad;
	GLuint colour_uniform, model_matrix_uniform;
};

class TextLabel : public Panel
{
public:
	TextLabel(float x, float y, Vector4F colour, std::optional<Vector4F> background_colour, std::optional<Vector3F> text_border_colour, Font font, const std::string& text, const Shader& shader);
	
	virtual void update() override;
	virtual float get_window_pos_x() const override;
	virtual float get_window_pos_y() const override;
	bool has_background_colour() const;
	bool has_text_border_colour() const;
	const Font& get_font() const;
	void set_font(Font font);
	const std::string& get_text() const;
	void set_text(const std::string& new_text);
	const Texture& get_texture() const;
	void set_texture(Texture texture);
	GLuint get_background_colour_uniform() const;
	GLuint get_has_background_colour_uniform() const;
	GLuint get_has_text_border_colour_uniform() const;
private:
	std::optional<Vector4F> background_colour;
	std::optional<Vector3F> text_border_colour;
	Font font;
	std::string text;
	Texture text_texture;
	GLuint background_colour_uniform, has_background_colour_uniform, text_border_colour_uniform, has_text_border_colour_uniform;
};

class Button : public TextLabel
{
public:
	Button(float x, float y, Vector4F colour, std::optional<Vector4F> background_colour, std::optional<Vector3F> text_border_colour, Font font, const std::string& text, const Shader& shader, MouseListener& mouse_listener);
	
	virtual void update() override;
	virtual bool focused() const override;
	virtual bool is_mouse_sensitive() const override;
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

#endif