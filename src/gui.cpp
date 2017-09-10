#include "gui.hpp"
#include "SDL_mixer.h"
#include "graphics.hpp"

GUIElement::GUIElement(): parent(nullptr), children(std::unordered_set<GUIElement*>()){}

GUIElement* GUIElement::getParent() const
{
	return this->parent;
}

GUIElement*& GUIElement::getParentR()
{
	return this->parent;
}

const std::unordered_set<GUIElement*>& GUIElement::getChildren() const
{
	return this->children;
}

std::unordered_set<GUIElement*>& GUIElement::getChildrenR()
{
	return this->children;
}

void GUIElement::addChild(GUIElement* child)
{
	this->children.insert(child);
	child->getParentR() = this;
}

bool GUIElement::isHidden() const
{
	return this->hidden;
}

void GUIElement::setHidden(bool hidden)
{
	this->hidden = hidden;
}

Window::Window(int w, int h, std::string title): GUIElement(), w(w), h(h), title(std::move(title)), is_close_requested(false)
{
	this->initSDL();
	this->initGLEW();
}

Window::Window(const Window& copy): Window(copy.w, copy.h, copy.title){}

Window::~Window()
{
	this->destSDL();
	for(auto pair : registered_listeners)
		this->deregisterListener(*(pair.second));
}

void Window::destroy()
{
	for(GUIElement* child : this->children)
		child->destroy();
	this->requestClose();
}

bool Window::focused()
{
	return SDL_GetWindowFlags(this->sdl_window_pointer) | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS;
}

int Window::getWidth() const
{
	return this->w;
}

int Window::getHeight() const
{
	return this->h;
}

int& Window::getWidthR()
{
	return this->w;
}

int& Window::getHeightR()
{
	return this->h;
}

bool Window::isCloseRequested() const
{
	return this->is_close_requested;
}

void Window::setSwapIntervalType(Window::SwapIntervalType type) const
{
	SDL_GL_SetSwapInterval(static_cast<int>(type));
}

Window::SwapIntervalType Window::getSwapIntervalType() const
{
	return static_cast<SwapIntervalType>(SDL_GL_GetSwapInterval());
}

void Window::requestClose()
{
	this->is_close_requested = true;
}

void Window::setTitle(std::string new_title)
{
	this->title = new_title;
	SDL_SetWindowTitle(this->sdl_window_pointer, new_title.c_str());
}

void Window::setRenderTarget() const
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glViewport(0, 0, this->w, this->h);
}

SDL_Window*& Window::getWindowHandleR()
{
	return this->sdl_window_pointer;
}

void Window::clear(float r, float g, float b, float a) const
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::update()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_DEPTH_CLAMP);
	glDisable(GL_CULL_FACE);
	for(GUIElement* element : this->children)
		element->update();
	SDL_GL_SwapWindow(this->sdl_window_pointer);
	this->handleEvents();
}

void Window::registerListener(Listener& l)
{
	this->registered_listeners[l.getID()] = &l;
}

void Window::deregisterListener(Listener& l)
{
	// Check if it actually exists before trying to remove it.
	if(this->registered_listeners.find(l.getID()) == this->registered_listeners.end())
		return;
	this->registered_listeners.erase(l.getID());
}

bool tz::graphics::initialised = false;
bool tz::graphics::has_context = false;
void Window::initSDL()
{
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	this->sdl_window_pointer = SDL_CreateWindow((this->title).c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, this->w, this->h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	this->sdl_gl_context_handle = SDL_GL_CreateContext(this->sdl_window_pointer);
	this->setSwapIntervalType(Window::SwapIntervalType::IMMEDIATE_UPDATES);
	tz::graphics::has_context = true;
	if(!tz::graphics::initialised)
		tz::graphics::initialise();
}

void Window::initGLEW()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH_CLAMP);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}

void Window::destSDL()
{
	SDL_GL_DeleteContext(this->sdl_gl_context_handle);
	SDL_DestroyWindow(this->sdl_window_pointer);
}

void Window::handleEvents()
{
	SDL_Event evt;
	while(SDL_PollEvent(&evt))
	{
		for(auto& listener : this->registered_listeners)
			listener.second->handleEvents(evt);
		
		if(evt.type == SDL_QUIT)
			this->is_close_requested = true;
		if(evt.type == SDL_WINDOWEVENT)
		{
			if (evt.window.event == SDL_WINDOWEVENT_RESIZED || evt.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
			{
				SDL_GL_GetDrawableSize(this->sdl_window_pointer, &(this->w), &(this->h));
				//update the glVievwport, so that OpenGL know the new window size
				glViewport(0, 0, this->w, this->h);
			}
		}
	}
}

Panel::Panel(unsigned int x, unsigned int y, unsigned int width, unsigned int height): GUIElement(), is_focused(false), x(x), y(y), width(width), height(height), quad(tz::graphics::createQuad()){}

unsigned int Panel::getX() const
{
	return this->x;
}

unsigned int Panel::getY() const
{
	return this->y;
}

unsigned int Panel::getWidth() const
{
	return this->width;
}

unsigned int Panel::getHeight() const
{
	return this->height;
}

unsigned int& Panel::getXR()
{
	return this->x;
}

unsigned int& Panel::getYR()
{
	return this->y;
}

unsigned int& Panel::getWidthR()
{
	return this->width;
}

unsigned int& Panel::getHeightR()
{
	return this->height;
}

const Vector3F& Panel::getColour() const
{
	return this->colour;
}

Vector3F& Panel::getColourR()
{
	return this->colour;
}

void Panel::update()
{
	this->quad.render(false);
}

void Panel::destroy()
{
	
}

bool Panel::focused()
{
	return this->is_focused;
}

void Panel::setFocused(bool focused)
{
	this->is_focused = focused;
}