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
	virtual void update();
	virtual void destroy();
	virtual bool focused() const = 0;
	virtual bool isWindow() const = 0;
	virtual float getWindowPosX() const = 0;
	virtual float getWindowPosY() const = 0;
	const Window* findWindowParent() const;
	bool hasWindowParent() const;
	const std::optional<std::reference_wrapper<const Shader>> getShader() const;
	std::optional<std::reference_wrapper<const Shader>>& getShaderR();
	bool hasShader() const;
	GUIElement* getParent() const;
	GUIElement*& getParentR();
	const std::unordered_set<GUIElement*>& getChildren() const;
	std::unordered_set<GUIElement*>& getChildrenR();
	void addChild(GUIElement* child);
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
	// Can't have these because the GL_Context cant be changed to anything that isn't dangerous when move's destructor is invoked (why would we want to move/assign a window anyway?)
	Window(Window&& move) = delete;
	Window& operator=(const Window& rhs) = delete;
	~Window();
	virtual void destroy();
	virtual bool focused() const;
	virtual bool isWindow() const;
	virtual float getWindowPosX() const;
	virtual float getWindowPosY() const;
	enum class SwapIntervalType : int
	{
		LATE_SWAP_TEARING = -1,
		IMMEDIATE_UPDATES = 0,
		VSYNC = 1,
	};
	int getWidth() const;
	int getHeight() const;
	int& getWidthR();
	int& getHeightR();
	bool isCloseRequested() const;
	void setSwapIntervalType(SwapIntervalType type) const;
	SwapIntervalType getSwapIntervalType() const;
	void requestClose();
	void setTitle(std::string new_title);
	void setRenderTarget() const;
	SDL_Window*& getWindowHandleR();
	void clear(float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f) const;
	void update();
	void registerListener(Listener& l);
	void deregisterListener(Listener& l);
private:
	void initSDL();
	void initGLEW();
	void destSDL();
	void handleEvents();
	
	std::unordered_map<unsigned int, Listener*> registered_listeners;
	int w, h;
	std::string title;
	bool is_close_requested;
	SDL_Window* sdl_window_pointer;
	SDL_GLContext sdl_gl_context_handle;
};

class Panel : public GUIElement
{
public:
	Panel(float x, float y, float width, float height, Vector3F colour, const Shader& shader);
	float getX() const;
	float getY() const;
	virtual float getWindowPosX() const;
	virtual float getWindowPosY() const;
	float getWidth() const;
	float getHeight() const;
	float& getXR();
	float& getYR();
	float& getWidthR();
	float& getHeightR();
	const Vector3F& getColour() const;
	Vector3F& getColourR();
	
	virtual void update();
	virtual void destroy();
	virtual bool focused() const;
	virtual bool isWindow() const;
	void setUsingProportionalPositioning(bool use_proportional_positioning);
	bool isUsingProportionalPositioning() const;
	void setFocused(bool focused);
protected:
	bool is_focused, use_proportional_positioning;
	float x, y, width, height;
	Vector3F colour;
	Mesh quad;
	GLuint colour_uniform, model_matrix_uniform;
};

class TextLabel : public Panel
{
public:
	TextLabel(float x, float y, Vector3F colour, std::optional<Vector3F> background_colour, Font font, const std::string& text, const Shader& shader);
	virtual void update();
	virtual float getWindowPosX() const;
	virtual float getWindowPosY() const;
	bool hasBackgroundColour() const;
	const Font& getFont() const;
	Font& getFontR();
	const std::string& getText() const;
	void setText(const std::string& new_text);
	const Texture& getTexture() const;
	Texture& getTextureR();
	GLuint getBackgroundColourUniform() const;
	GLuint getHasBackgroundColourUniform() const;
private:
	std::optional<Vector3F>& background_colour;
	Font font;
	std::string text;
	Texture text_texture;
	GLuint background_colour_uniform, has_background_colour_uniform;
};

class Button : public TextLabel
{
public:
	Button(float x, float y, Vector3F colour, std::optional<Vector3F> background_colour, Font font, const std::string& text, const Shader& shader, MouseListener& mouse_listener);
	virtual void update();
	Command* getOnMouseOver() const;
	Command* getOnMouseClick() const;
	Command*& getOnMouseOverR();
	Command*& getOnMouseClickR();
	bool mousedOver() const;
	bool clickedOn() const;
private:
	MouseListener& mouse_listener;
	Command* on_mouse_over;
	Command* on_mouse_click;
};

#endif