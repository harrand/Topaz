#include "window.hpp"

// Static members need to be initialised like this
unsigned int Listener::NUM_LISTENERS = 0;

Listener::Listener()
{
	Listener::NUM_LISTENERS++;
	this->id = Listener::NUM_LISTENERS;
}

// Don't want it defaulting or this id will be the same as the copy without incrementing NUM_LISTENERS. Calling ctor works fine
Listener::Listener(const Listener& copy): Listener(){}

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

Window::Window(int w, int h, std::string title): w(w), h(h), title(title), iscloserequested(false)
{
	this->initSDL();
	this->initGLEW();
}

Window::Window(const Window& copy): Window(copy.w, copy.h, copy.title){}

Window::~Window()
{
	this->destSDL();
}

int& Window::getWidth()
{
	return this->w;
}

int& Window::getHeight()
{
	return this->h;
}

bool Window::isCloseRequested() const
{
	return this->iscloserequested;
}

void Window::requestClose()
{
	this->iscloserequested = true;
}

void Window::setTitle(std::string newTitle)
{
	this->title = newTitle;
	SDL_SetWindowTitle(this->wnd, newTitle.c_str());
}

void Window::setRenderTarget() const
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glViewport(0, 0, this->w, this->h);
}

void Window::clear(float r, float g, float b, float a) const
{
		glClearColor(r, g, b, a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::update()
{
	SDL_GL_SwapWindow(this->wnd);
	this->handleEvents();
}

void Window::registerListener(Listener& l)
{
	this->registeredListeners[l.getID()] = &l;
}

void Window::deregisterListener(Listener& l)
{
	// Check if it actually exists before trying to remove it.
	if(this->registeredListeners.find(l.getID()) == this->registeredListeners.end())
		return;
	this->registeredListeners.erase(l.getID());
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
	
	this->wnd = SDL_CreateWindow((this->title).c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, this->w, this->h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	this->ctx = SDL_GL_CreateContext(this->wnd);
	SDL_GL_SetSwapInterval(0);
	
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
	SDL_GL_DeleteContext(this->ctx);
	SDL_DestroyWindow(this->wnd);
	SDL_Quit();
}

void Window::handleEvents()
{
	SDL_Event evt;
	while(SDL_PollEvent(&evt))
	{
		for(auto& listener : this->registeredListeners)
			listener.second->handleEvents(evt);
		
		if(evt.type == SDL_QUIT)
			this->iscloserequested = true;
		else if(evt.type == SDL_WINDOWEVENT)
		{
			if (evt.window.event == (SDL_WINDOWEVENT_RESIZED | SDL_WINDOWEVENT_SIZE_CHANGED))
			{
				SDL_GetWindowSize(this->wnd, &(this->w), &(this->h));
				//update the glVievwport, so that OpenGL know the new window size
				glViewport(0, 0, this->w, this->h);
			}
		}
	}
}