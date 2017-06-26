#ifndef WINDOW_HPP
#define WINDOW_HPP
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <string>
#include <unordered_map>
#include "SDL.h"

class Listener
{
public:
	Listener();
	Listener(const Listener& copy) = delete;
	Listener(Listener&& move) = delete;
	Listener& operator=(const Listener& rhs) = delete;
	~Listener();
	
	virtual void handleEvents(SDL_Event& evt) = 0;
	unsigned int getID() const;
	static unsigned int getNumListeners();
private:
	static unsigned int NUM_LISTENERS;
	unsigned int id;
};

class Window
{
public:
	Window(int w = 800, int h = 600, std::string title = "Untitled");
	Window(const Window& copy);
	// Can't have these because the GL_Context cant be changed to anything that isn't dangerous when move's destructor is invoked (why would we want to move/assign a window anyway?)
	Window(Window&& move) = delete;
	Window& operator=(const Window& rhs) = delete;
	~Window();
	
	int getWidth() const;
	int getHeight() const;
	int& getWidthR();
	int& getHeightR();
	bool isCloseRequested() const;
	void requestClose();
	void setTitle(std::string newTitle);
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
	std::unordered_map<unsigned int, Listener*> registeredListeners;
	
	int w, h;
	std::string title;
	bool iscloserequested;
	SDL_Window* wnd;
	SDL_GLContext ctx;
};

#endif