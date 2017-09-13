#ifndef GUI_HPP
#define GUI_HPP
#include <optional>
#include <functional>
#include <string>
#include <unordered_map>
#include "SDL.h"
#include "SDL_ttf.h"
#include "listeners.hpp"
#include "vector.hpp"
#include "mesh.hpp"

class Window;

class GUIElement
{
public:
	GUIElement(std::optional<std::reference_wrapper<const Shader>> shader);
	virtual void update() = 0;
	virtual void destroy() = 0;
	virtual bool focused() const = 0;
	virtual bool isWindow() const = 0;
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
	void setFocused(bool focused);
protected:
	bool is_focused;
	float x, y, width, height;
	Vector3F colour;
	Mesh quad;
	GLuint colour_uniform, model_matrix_uniform;
};

class TextField : public Panel
{
public:
	TextField(float x, float y, float width, float height, Vector3F colour, TTF_Font* font, const std::string& text, const Shader& shader);
	virtual void update();
	const std::string& getText() const;
	void setText(const std::string& new_text);
private:
	TTF_Font* font;
	std::string text;
	Texture text_texture;
};

#endif