#include "window.hpp"
#include "SDL_mixer.h"
#include "glew.h"

// Static objects need to be initialised like this
unsigned int Listener::NUM_LISTENERS = 0;

Listener::Listener()
{
	Listener::NUM_LISTENERS++;
	this->id = Listener::NUM_LISTENERS;
}

Listener::~Listener()
{
	Listener::NUM_LISTENERS--;
}

unsigned int Listener::getID() const
{
	return this->id;
}

// static
unsigned int Listener::getNumListeners()
{
	return Listener::NUM_LISTENERS;
}

Window::Window(int w, int h, std::string title): w(w), h(h), title(std::move(title)), is_close_requested(false)
{
	this->initSDL();
	this->initGLEW();
}

Window::Window(const Window& copy): Window(copy.w, copy.h, copy.title){}

Window::~Window()
{
	this->destSDL();
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

void Window::initSDL()
{
	SDL_Init(SDL_INIT_EVERYTHING);
	
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
	
	//Initialise SDL_mixer
	Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);
}

void Window::initGLEW()
{
	GLenum status = glewInit();
	if(status != GLEW_OK)
	{
		this->title = "Fatal Error: GLEW Initialisation Failed";
	}
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH_CLAMP);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}

void Window::destSDL()
{
	Mix_CloseAudio();
	SDL_GL_DeleteContext(this->sdl_gl_context_handle);
	SDL_DestroyWindow(this->sdl_window_pointer);
	SDL_Quit();
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