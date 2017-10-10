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
	virtual bool isWindow() const = 0;
	virtual bool isMouseSensitive() const = 0;
	virtual float getWindowPosX() const = 0;
	virtual float getWindowPosY() const = 0;
	Window* findWindowParent() const;
	bool hasWindowParent() const;
	const std::optional<std::reference_wrapper<const Shader>> getShader() const;
	bool hasShader() const;
	GUIElement* getParent() const;
	void setParent(GUIElement* parent);
	const std::unordered_set<GUIElement*>& getChildren() const;
	void addChild(GUIElement* child);
	void removeChild(GUIElement* child);
	bool isHidden() const;
	void setHidden(bool hidden);
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
	virtual bool isWindow() const override;
	virtual bool isMouseSensitive() const override;
	virtual float getWindowPosX() const override;
	virtual float getWindowPosY() const override;
	enum class SwapIntervalType : int
	{
		LATE_SWAP_TEARING = -1,
		IMMEDIATE_UPDATES = 0,
		VSYNC = 1,
	};
	int getWidth() const;
	int getHeight() const;
	void setWidth(float width);
	void setHeight(float height);
	bool isCloseRequested() const;
	void setSwapIntervalType(SwapIntervalType type) const;
	SwapIntervalType getSwapIntervalType() const;
	void setTitle(const std::string& new_title);
	void setRenderTarget() const;
	void clearFocus();
	SDL_Window* getWindowHandle() const;
	void clear(float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f) const;
	void registerListener(Listener& l);
	void deregisterListener(Listener& l);
	GUIElement* getFocusedChild() const;
	void setFocusedChild(GUIElement* child);
private:	
	std::unordered_map<unsigned int, Listener*> registered_listeners;
	int w, h;
	std::string title;
	bool is_close_requested;
	SDL_Window* sdl_window_pointer;
	SDL_GLContext sdl_gl_context_handle;
	GUIElement* focused_child;
};

class Panel : public GUIElement
{
public:
	Panel(float x, float y, float width, float height, Vector4F colour, const Shader& shader);
	
	float getX() const;
	float getY() const;
	virtual float getWindowPosX() const override;
	virtual float getWindowPosY() const override;
	float getWidth() const;
	float getHeight() const;
	const Vector4F& getColour() const;
	void setX(float x);
	void setY(float y);
	void setWidth(float width);
	void setHeight(float height);
	void setColour(Vector4F colour);
	virtual void update() override;
	virtual void destroy() override;
	virtual bool focused() const override;
	virtual bool isWindow() const override;
	virtual bool isMouseSensitive() const override;
	void setUsingProportionalPositioning(bool use_proportional_positioning);
	bool isUsingProportionalPositioning() const;
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
	virtual float getWindowPosX() const override;
	virtual float getWindowPosY() const override;
	bool hasBackgroundColour() const;
	bool hasTextBorderColour() const;
	const Font& getFont() const;
	void setFont(Font font);
	const std::string& getText() const;
	void setText(const std::string& new_text);
	const Texture& getTexture() const;
	void setTexture(Texture texture);
	GLuint getBackgroundColourUniform() const;
	GLuint getHasBackgroundColourUniform() const;
	GLuint getHasTextBorderColourUniform() const;
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
	virtual bool isMouseSensitive() const override;
	Command* getOnMouseOver() const;
	Command* getOnMouseClick() const;
	void setOnMouseOver(Command* cmd);
	void setOnMouseClick(Command* cmd);
	bool mousedOver() const;
	bool clickedOn() const;
protected:
	MouseListener& mouse_listener;
	bool just_clicked, just_moused_over;
private:
	Command* on_mouse_over;
	Command* on_mouse_click;
};

#endif