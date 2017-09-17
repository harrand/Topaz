#include "gui.hpp"
#include "SDL_mixer.h"
#include "graphics.hpp"

GUIElement::GUIElement(std::optional<std::reference_wrapper<const Shader>> shader): shader(shader), parent(nullptr), children(std::unordered_set<GUIElement*>()), hidden(false){}

void GUIElement::update()
{
	for(GUIElement* child : this->children)
		child->update();
}

void GUIElement::destroy()
{
	for(GUIElement* child : this->children)
		child->destroy();
}

const Window* GUIElement::findWindowParent() const
{
	const GUIElement* res = this;
	while(res != nullptr && !res->isWindow())
		res = res->getParent();
	return dynamic_cast<const Window*>(res);
}

bool GUIElement::hasWindowParent() const
{
	return this->findWindowParent() != nullptr;
}

const std::optional<std::reference_wrapper<const Shader>> GUIElement::getShader() const
{
	return this->shader;
}

bool GUIElement::hasShader() const
{
	return this->shader.has_value();
}

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

bool tz::graphics::initialised = false;
bool tz::graphics::has_context = false;

Window::Window(int w, int h, std::string title): GUIElement({}), w(w), h(h), title(std::move(title)), is_close_requested(false)
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
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH_CLAMP);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}

Window::Window(const Window& copy): Window(copy.w, copy.h, copy.title){}

Window::~Window()
{
	for(auto pair : registered_listeners)
		if(pair.first < Listener::getNumListeners()) // checks to make sure the listener hasnt gone out of scope
			this->deregisterListener(*(pair.second));
	SDL_GL_DeleteContext(this->sdl_gl_context_handle);
	SDL_DestroyWindow(this->sdl_window_pointer);
}

void Window::update()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_DEPTH_CLAMP);
	glDisable(GL_CULL_FACE);
	GUIElement::update();
	SDL_GL_SwapWindow(this->sdl_window_pointer);
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

void Window::destroy()
{
	GUIElement::destroy();
	this->is_close_requested = true;
}

bool Window::focused() const
{
	return SDL_GetWindowFlags(this->sdl_window_pointer) | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS;
}

bool Window::isWindow() const
{
	return true;
}

float Window::getWindowPosX() const
{
	return 0;
}

float Window::getWindowPosY() const
{
	return 0;
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

void Window::setTitle(const std::string& new_title)
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

Panel::Panel(float x, float y, float width, float height, Vector4F colour, const Shader& shader): GUIElement(shader), use_proportional_positioning(false), x(x), y(y), width(width), height(height), colour(colour), quad(tz::graphics::createQuad()), colour_uniform(glGetUniformLocation(this->shader.value().get().getProgramHandle(), "colour")), model_matrix_uniform(glGetUniformLocation(this->shader.value().get().getProgramHandle(), "model_matrix")){}

float Panel::getX() const
{
	return this->x;
}

float Panel::getY() const
{
	return this->y;
}

float Panel::getWindowPosX() const
{
	float offset = 0;
	if(this->parent != nullptr)
		offset = this->parent->getWindowPosX();
	return offset + (this->x + this->width);
}

float Panel::getWindowPosY() const
{
	float offset = 0;
	if(this->parent != nullptr)
		offset = this->parent->getWindowPosY();
	return offset + (this->y + this->height);
}

float Panel::getWidth() const
{
	return this->width;
}

float Panel::getHeight() const
{
	return this->height;
}

float& Panel::getXR()
{
	return this->x;
}

float& Panel::getYR()
{
	return this->y;
}

float& Panel::getWidthR()
{
	return this->width;
}

float& Panel::getHeightR()
{
	return this->height;
}

const Vector4F& Panel::getColour() const
{
	return this->colour;
}

Vector4F& Panel::getColourR()
{
	return this->colour;
}

void Panel::update()
{
	if(!this->hidden)
	{
		this->shader.value().get().bind();
		glUniform1i(glGetUniformLocation(this->shader.value().get().getProgramHandle(), "has_texture"), false);
		glUniform1i(glGetUniformLocation(this->shader.value().get().getProgramHandle(), "has_background_colour"), false);
		glUniform1i(glGetUniformLocation(this->shader.value().get().getProgramHandle(), "has_text_border_colour"), false);
		glUniform4f(this->colour_uniform, this->colour.getX(), this->colour.getY(), this->colour.getZ(), this->colour.getW());
		Matrix4x4 projection;
		if(this->hasWindowParent() && !this->use_proportional_positioning)
			projection = Matrix4x4::createOrthographicMatrix(this->findWindowParent()->getWidth(), 0.0f, this->findWindowParent()->getHeight(), 0.0f, -1.0f, 1.0f);
		else
			projection = Matrix4x4::identity();
		glUniformMatrix4fv(this->model_matrix_uniform, 1, GL_TRUE, (projection * Matrix4x4::createModelMatrix(Vector3F(this->getWindowPosX(), this->getWindowPosY(), 0.0f), Vector3F(), Vector3F(this->width, this->height, 0.0f))).fillData().data());
		this->quad.render(false);
		GUIElement::update();
	}
}

void Panel::destroy()
{
	GUIElement::destroy();
	if(this->parent != nullptr)
	{
		this->parent->getChildrenR().erase(this);
		this->parent = nullptr;
	}
}

bool Panel::focused() const
{
	if(!this->hasWindowParent())
		return false;
	return this->findWindowParent()->focused();
}

bool Panel::isWindow() const
{
	return false;
}

void Panel::setUsingProportionalPositioning(bool use_proportional_positioning)
{
	this->use_proportional_positioning = use_proportional_positioning;
}

bool Panel::isUsingProportionalPositioning() const
{
	return this->use_proportional_positioning;
}

TextLabel::TextLabel(float x, float y, Vector4F colour, std::optional<Vector4F> background_colour, std::optional<Vector3F> text_border_colour, Font font, const std::string& text, const Shader& shader): Panel(x, y, this->text_texture.getWidth(), this->text_texture.getHeight(), colour, shader), background_colour(background_colour), text_border_colour(text_border_colour), font(font), text(text), text_texture(this->font.getTTFR(), this->text, SDL_Color({static_cast<unsigned char>(this->colour.getX() * 255), static_cast<unsigned char>(this->colour.getY() * 255), static_cast<unsigned char>(this->colour.getZ() * 255), static_cast<unsigned char>(255)})), background_colour_uniform(glGetUniformLocation(this->shader.value().get().getProgramHandle(), "background_colour")), has_background_colour_uniform(glGetUniformLocation(this->shader.value().get().getProgramHandle(), "has_background_colour")), text_border_colour_uniform(glGetUniformLocation(this->shader.value().get().getProgramHandle(), "text_border_colour")), has_text_border_colour_uniform(glGetUniformLocation(this->shader.value().get().getProgramHandle(), "has_text_border_colour"))
{
	// Not in initialiser list because text_texture MUST be initialised after Panel, and theres no way of initialising it before without a warning so do it here.
	this->width = text_texture.getWidth();
	this->height = text_texture.getHeight();
}

void TextLabel::update()
{
	if(!this->hidden)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		this->shader.value().get().bind();
		glUniform1i(glGetUniformLocation(this->shader.value().get().getProgramHandle(), "has_texture"), true);
		glUniform1i(this->has_background_colour_uniform, this->hasBackgroundColour() ? true : false);
		glUniform1i(this->has_text_border_colour_uniform, this->hasTextBorderColour() ? true : false);
		if(this->hasBackgroundColour())
			glUniform4f(this->background_colour_uniform, this->background_colour.value().getX(), this->background_colour.value().getY(), this->background_colour.value().getZ(), this->background_colour.value().getW());
		if(this->hasTextBorderColour())
			glUniform3f(this->text_border_colour_uniform, this->text_border_colour.value().getX(), this->text_border_colour.value().getY(), this->text_border_colour.value().getZ());
		Matrix4x4 projection;
		if(this->hasWindowParent() && !this->use_proportional_positioning)
			projection = Matrix4x4::createOrthographicMatrix(this->findWindowParent()->getWidth(), 0.0f, this->findWindowParent()->getHeight(), 0.0f, -1.0f, 1.0f);
		else
			projection = Matrix4x4::identity();
		glUniformMatrix4fv(this->model_matrix_uniform, 1, GL_TRUE, (projection * Matrix4x4::createModelMatrix(Vector3F(this->getWindowPosX(), this->getWindowPosY(), 0.0f), Vector3F(), Vector3F(this->width, this->height, 0.0f))).fillData().data());
		this->text_texture.bind(this->shader.value().get().getProgramHandle(), 0);
		this->quad.render(false);
		GUIElement::update();
	}
}

float TextLabel::getWindowPosX() const
{
	float offset = 0;
	if(this->parent != nullptr && !this->parent->isWindow())
		offset = dynamic_cast<Panel*>(this->parent)->getX();
	return offset + (this->x + this->width);
}

float TextLabel::getWindowPosY() const
{
	float offset = 0;
	if(this->parent != nullptr && !this->parent->isWindow())
		offset = dynamic_cast<Panel*>(this->parent)->getY();
	return offset + (this->y + this->height);
}

bool TextLabel::hasBackgroundColour() const
{
	return this->background_colour.has_value();
}

bool TextLabel::hasTextBorderColour() const
{
	return this->text_border_colour.has_value();
}

const Font& TextLabel::getFont() const
{
	return this->font;
}

Font& TextLabel::getFontR()
{
	return this->font;
}

const std::string& TextLabel::getText() const
{
	return this->text;
}
void TextLabel::setText(const std::string& new_text)
{
	this->text = new_text;
	this->text_texture = Texture(this->font.getTTFR(), this->text, SDL_Color({static_cast<unsigned char>(this->colour.getX() * 255), static_cast<unsigned char>(this->colour.getY() * 255), static_cast<unsigned char>(this->colour.getZ() * 255), static_cast<unsigned char>(255)}));
	this->width = text_texture.getWidth();
	this->height = text_texture.getHeight();
}

const Texture& TextLabel::getTexture() const
{
	return this->text_texture;
}

Texture& TextLabel::getTextureR()
{
	return this->text_texture;
}

GLuint TextLabel::getBackgroundColourUniform() const
{
	return this->background_colour_uniform;
}

GLuint TextLabel::getHasBackgroundColourUniform() const
{
	return this->has_background_colour_uniform;
}

GLuint TextLabel::getHasTextBorderColourUniform() const
{
	return this->has_text_border_colour_uniform;
}

Button::Button(float x, float y, Vector4F colour, std::optional<Vector4F> background_colour, std::optional<Vector3F> text_border_colour, Font font, const std::string& text, const Shader& shader, MouseListener& mouse_listener): TextLabel(x, y, colour, background_colour, text_border_colour, font, text, shader), mouse_listener(mouse_listener), on_mouse_over(nullptr), on_mouse_click(nullptr), just_clicked(false), just_moused_over(false){}

void Button::update()
{
	if(this->clickedOn() && this->on_mouse_click != nullptr && !this->just_clicked)
	{
		this->on_mouse_click->operator()({});
		this->just_clicked = true;
	}
	else if(!this->clickedOn())
		this->just_clicked = false;
	TextLabel::update();
}

Command* Button::getOnMouseOver() const
{
	return this->on_mouse_over;
}

Command* Button::getOnMouseClick() const
{
	return this->on_mouse_click;
}

Command*& Button::getOnMouseOverR()
{
	return this->on_mouse_over;
}

Command*& Button::getOnMouseClickR()
{
	return this->on_mouse_click;
}

bool Button::mousedOver() const
{
	Vector2F mouse_pos = this->mouse_listener.getMousePos();
	bool x_aligned = mouse_pos.getX() >= (this->getWindowPosX() - this->width) && mouse_pos.getX() <= (this->getWindowPosX() + this->width);
	bool y_aligned = mouse_pos.getY() >= (this->findWindowParent()->getHeight() - this->getWindowPosY() - this->height) && mouse_pos.getY() <= ((this->findWindowParent()->getHeight() - this->getWindowPosY() + this->height));
	return x_aligned && y_aligned;
}

bool Button::clickedOn() const
{
	return this->mousedOver() && this->mouse_listener.isLeftClicked();
}