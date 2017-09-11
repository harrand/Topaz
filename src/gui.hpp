#ifndef GUI_HPP
#define GUI_HPP
#include <string>
#include <unordered_map>
#include "SDL.h"
#include "listeners.hpp"
#include "vector.hpp"
#include "mesh.hpp"

class GUIElement
{
public:
	GUIElement();
	virtual void update() = 0;
	virtual void destroy() = 0;
	virtual bool focused() = 0;
	GUIElement* getParent() const;
	GUIElement*& getParentR();
	const std::unordered_set<GUIElement*>& getChildren() const;
	std::unordered_set<GUIElement*>& getChildrenR();
	void addChild(GUIElement* child);
	bool isHidden() const;
	void setHidden(bool hidden);
protected:
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
	virtual bool focused();
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
	Panel(float x, float y, float width, float height);
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
	virtual bool focused();
	void setFocused(bool focused);
protected:
	bool is_focused;
private:
	float x, y, width, height;
	Vector3F colour;
	Mesh quad;
};

#endif