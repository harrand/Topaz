#ifndef WINDOW_HPP
#define WINDOW_HPP
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <string>
#include <map>
#include "SDL.h"
#include "glew.h"

class Listener
{
public:
	Listener();
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
	Window(int w = 800, int h = 600, std::string title = "Undefined");
	~Window();
	int& getWidth();
	int& getHeight();
	bool isCloseRequested() const;
	void requestClose();
	void setTitle(std::string newTitle);
	
	void clear(float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f) const;
	void update();
	
	void registerListener(Listener& l);
	void deregisterListener(Listener& l);
	
private:
	bool iscloserequested;
	void initSDL();
	void initGLEW();
	void destSDL();
	void handleEvents();
	std::map<unsigned int, Listener*> registeredListeners;
	int w, h;
	std::string title;
	
	SDL_Window* wnd;
	SDL_GLContext ctx;
};

#endif