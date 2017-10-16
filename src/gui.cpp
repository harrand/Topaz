#include "gui.hpp"
#include "SDL_mixer.h"
#include "graphics.hpp"

GUIElement::GUIElement(std::optional<std::reference_wrapper<const Shader>> shader): shader(shader), parent(nullptr), children(std::unordered_set<GUIElement*>()), hidden(false){}

void GUIElement::update() // updates all children
{
	for(GUIElement* child : this->children)
		child->update();
}

void GUIElement::destroy() // destroys all children
{
	for(GUIElement* child : this->children)
		child->destroy();
}

Window* GUIElement::find_window_parent() const
{
	// returns a pointer to the first ancestor which is a window. if there is no such ancestor, return nullptr
	GUIElement* res = this->get_parent();
	while(res != nullptr && !res->is_window())
		res = res->get_parent();
	return dynamic_cast<Window*>(res);
}

bool GUIElement::has_window_parent() const
{
	return this->find_window_parent() != nullptr;
}

const std::optional<std::reference_wrapper<const Shader>> GUIElement::get_shader() const
{
	return this->shader;
}

bool GUIElement::has_shader() const
{
	return this->shader.has_value();
}

GUIElement* GUIElement::get_parent() const
{
	return this->parent;
}

void GUIElement::set_parent(GUIElement* parent)
{
	this->parent = parent;
}

const std::unordered_set<GUIElement*>& GUIElement::get_children() const
{
	return this->children;
}

void GUIElement::add_child(GUIElement* child)
{
	this->children.insert(child);
	child->set_parent(this);
}

void GUIElement::remove_child(GUIElement* child)
{
	this->children.erase(child);
	child->set_parent(nullptr);
}

bool GUIElement::is_hidden() const
{
	return this->hidden;
}

void GUIElement::set_hidden(bool hidden)
{
	this->hidden = hidden;
}

bool tz::graphics::initialised = false;
bool tz::graphics::has_context = false;

Window::Window(int w, int h, std::string title): GUIElement({}), w(w), h(h), title(std::move(title)), close_requested(false), focused_child(nullptr)
{
	// setup attributes for the gl_context
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32); // number of bits per pixel (should be total of rgba size)
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24); // 24-bit depth-buffer. was originally using 16-bit but i found that it's not quite enough for my liking. 24 bits is plenty.
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	// msaa, one buffer, 4 samples
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
	
	this->sdl_window_pointer = SDL_CreateWindow((this->title).c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, this->w, this->h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	this->sdl_gl_context_handle = SDL_GL_CreateContext(this->sdl_window_pointer);
	this->set_swap_interval_type(Window::SwapIntervalType::IMMEDIATE_UPDATES);
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
		if(pair.first < Listener::get_num_listeners()) // checks to make sure the listener hasnt gone out of scope
			this->deregister_listener(*(pair.second));
	SDL_GL_DeleteContext(this->sdl_gl_context_handle);
	SDL_DestroyWindow(this->sdl_window_pointer);
}

void Window::update()
{
	// temporarily disable all the 3d features whilst rendering GUI (we don't want a depth buffer culling gui)
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_DEPTH_CLAMP);
	glDisable(GL_CULL_FACE);
	GUIElement::update();
	SDL_GL_SwapWindow(this->sdl_window_pointer);
	SDL_Event evt;
	while(SDL_PollEvent(&evt))
	{
		for(auto& listener : this->registered_listeners)
			listener.second->handle_events(evt);
		
		if(evt.type == SDL_QUIT)
			this->close_requested = true;
		if(evt.type == SDL_WINDOWEVENT)
		{
			if (evt.window.event == SDL_WINDOWEVENT_RESIZED || evt.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
			{
				SDL_GL_GetDrawableSize(this->sdl_window_pointer, &(this->w), &(this->h));
				//update the glViewport, so that OpenGL knows the new window size
				glViewport(0, 0, this->w, this->h);
			}
		}
	}
}

void Window::destroy()
{
	GUIElement::destroy();
	this->close_requested = true;
}

bool Window::focused() const
{
	return SDL_GetWindowFlags(this->sdl_window_pointer) | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS;
}

bool Window::is_window() const
{
	return true;
}

bool Window::is_mouse_sensitive() const
{
	return false;
}

float Window::get_window_pos_x() const
{
	return 0;
}

float Window::get_window_pos_y() const
{
	return 0;
}

int Window::get_width() const
{
	return this->w;
}

int Window::get_height() const
{
	return this->h;
}

void Window::set_width(float width)
{
	this->w = width;
}

void Window::set_height(float height)
{
	this->h = height;
}

bool Window::is_close_requested() const
{
	return this->close_requested;
}

void Window::set_swap_interval_type(Window::SwapIntervalType type) const
{
	SDL_GL_SetSwapInterval(static_cast<int>(type));
}

Window::SwapIntervalType Window::get_swap_interval_type() const
{
	return static_cast<SwapIntervalType>(SDL_GL_GetSwapInterval());
}

void Window::set_title(const std::string& new_title)
{
	this->title = new_title;
	SDL_SetWindowTitle(this->sdl_window_pointer, new_title.c_str());
}

void Window::set_render_target() const
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glViewport(0, 0, this->w, this->h);
}

void Window::clear_focus()
{
	this->focused_child = nullptr;
}

SDL_Window* Window::get_window_handle() const
{
	return this->sdl_window_pointer;
}

void Window::clear(float r, float g, float b, float a) const
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::register_listener(Listener& l)
{
	this->registered_listeners[l.get_id()] = &l;
}

void Window::deregister_listener(Listener& l)
{
	// Check if it actually exists before trying to remove it.
	if(this->registered_listeners.find(l.get_id()) == this->registered_listeners.end())
		return;
	this->registered_listeners.erase(l.get_id());
}

GUIElement* Window::get_focused_child() const
{
	return this->focused_child;
}

void Window::set_focused_child(GUIElement* child)
{
	this->focused_child = child;
}

Panel::Panel(float x, float y, float width, float height, Vector4F colour, const Shader& shader): GUIElement(shader), use_proportional_positioning(false), x(x), y(y), width(width), height(height), colour(colour), quad(tz::graphics::create_quad()), colour_uniform(glGetUniformLocation(this->shader.value().get().get_program_handle(), "colour")), model_matrix_uniform(glGetUniformLocation(this->shader.value().get().get_program_handle(), "model_matrix")){}

float Panel::get_x() const
{
	return this->x;
}

float Panel::get_y() const
{
	return this->y;
}

float Panel::get_window_pos_x() const
{
	float offset = 0;
	if(this->parent != nullptr)
		offset = this->parent->get_window_pos_x(); // recursive. offset = total offsets of all ancestors to get true window pos
	return offset + (this->x + this->width);
}

float Panel::get_window_pos_y() const
{
	float offset = 0;
	if(this->parent != nullptr)
		offset = this->parent->get_window_pos_y(); // recursive. offset = total offsets of all ancestors to get true window pos
	return offset + (this->y + this->height);
}

float Panel::get_width() const
{
	return this->width;
}

float Panel::get_height() const
{
	return this->height;
}

const Vector4F& Panel::get_colour() const
{
	return this->colour;
}

void Panel::set_x(float x)
{
	this->x = x;
}

void Panel::set_y(float y)
{
	this->y = y;
}

void Panel::set_width(float width)
{
	this->width = width;
}

void Panel::set_height(float height)
{
	this->height = height;
}

void Panel::set_colour(Vector4F colour)
{
	this->colour = colour;
}

void Panel::update()
{
	if(!this->hidden)
	{
		//update uniforms & bind & render. THEN update all children (unless the panel is hidden in which case do nothing)
		this->shader.value().get().bind();
		glUniform1i(glGetUniformLocation(this->shader.value().get().get_program_handle(), "has_texture"), false);
		glUniform1i(glGetUniformLocation(this->shader.value().get().get_program_handle(), "has_background_colour"), false);
		glUniform1i(glGetUniformLocation(this->shader.value().get().get_program_handle(), "has_text_border_colour"), false);
		glUniform4f(this->colour_uniform, this->colour.get_x(), this->colour.get_y(), this->colour.get_z(), this->colour.get_w());
		Matrix4x4 projection;
		if(this->has_window_parent() && !this->use_proportional_positioning)
			projection = Matrix4x4::create_orthographic_matrix(this->find_window_parent()->get_width(), 0.0f, this->find_window_parent()->get_height(), 0.0f, -1.0f, 1.0f);
		else
			projection = Matrix4x4::identity();
		glUniformMatrix4fv(this->model_matrix_uniform, 1, GL_TRUE, (projection * Matrix4x4::create_model_matrix(Vector3F(this->get_window_pos_x(), this->get_window_pos_y(), 0.0f), Vector3F(), Vector3F(this->width, this->height, 0.0f))).fill_data().data());
		this->quad.render(false);
		GUIElement::update();
	}
}

void Panel::destroy()
{
	// kill all children before killing itself (that was a dark comment)
	GUIElement::destroy();
	if(this->parent != nullptr)
	{
		this->parent->remove_child(this);
		this->parent = nullptr;
	}
}

bool Panel::focused() const
{
	return false;
}

bool Panel::is_window() const
{
	return false;
}

bool Panel::is_mouse_sensitive() const
{
	return false;
}

void Panel::set_using_proportional_positioning(bool use_proportional_positioning)
{
	this->use_proportional_positioning = use_proportional_positioning;
}

bool Panel::is_using_proportional_positioning() const
{
	return this->use_proportional_positioning;
}

TextLabel::TextLabel(float x, float y, Vector4F colour, std::optional<Vector4F> background_colour, std::optional<Vector3F> text_border_colour, Font font, const std::string& text, const Shader& shader): Panel(x, y, this->text_texture.get_width(), this->text_texture.get_height(), colour, shader), background_colour(background_colour), text_border_colour(text_border_colour), font(font), text(text), text_texture(this->font, this->text, SDL_Color({static_cast<unsigned char>(this->colour.get_x() * 255), static_cast<unsigned char>(this->colour.get_y() * 255), static_cast<unsigned char>(this->colour.get_z() * 255), static_cast<unsigned char>(255)})), background_colour_uniform(glGetUniformLocation(this->shader.value().get().get_program_handle(), "background_colour")), has_background_colour_uniform(glGetUniformLocation(this->shader.value().get().get_program_handle(), "has_background_colour")), text_border_colour_uniform(glGetUniformLocation(this->shader.value().get().get_program_handle(), "text_border_colour")), has_text_border_colour_uniform(glGetUniformLocation(this->shader.value().get().get_program_handle(), "has_text_border_colour"))
{
	// Not in initialiser list because text_texture MUST be initialised after Panel, and theres no way of initialising it before without a warning so do it here.
	this->width = text_texture.get_width();
	this->height = text_texture.get_height();
}

void TextLabel::update()
{
	if(!this->hidden)
	{
		// enable blending as the glyphs from TTF fonts have alot of alpha variation which requires blending to look in any-way legible
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		this->shader.value().get().bind();
		glUniform1i(glGetUniformLocation(this->shader.value().get().get_program_handle(), "has_texture"), true);
		glUniform1i(this->has_background_colour_uniform, this->has_background_colour() ? true : false);
		glUniform1i(this->has_text_border_colour_uniform, this->has_text_border_colour() ? true : false);
		if(this->has_background_colour())
			glUniform4f(this->background_colour_uniform, this->background_colour.value().get_x(), this->background_colour.value().get_y(), this->background_colour.value().get_z(), this->background_colour.value().get_w());
		if(this->has_text_border_colour())
			glUniform3f(this->text_border_colour_uniform, this->text_border_colour.value().get_x(), this->text_border_colour.value().get_y(), this->text_border_colour.value().get_z());
		Matrix4x4 projection;
		if(this->has_window_parent() && !this->use_proportional_positioning)
			projection = Matrix4x4::create_orthographic_matrix(this->find_window_parent()->get_width(), 0.0f, this->find_window_parent()->get_height(), 0.0f, -1.0f, 1.0f);
		else
			projection = Matrix4x4::identity();
		glUniformMatrix4fv(this->model_matrix_uniform, 1, GL_TRUE, (projection * Matrix4x4::create_model_matrix(Vector3F(this->get_window_pos_x(), this->get_window_pos_y(), 0.0f), Vector3F(), Vector3F(this->width, this->height, 0.0f))).fill_data().data());
		this->text_texture.bind(this->shader.value().get().get_program_handle(), 0);
		this->quad.render(false);
		GUIElement::update();
	}
}

float TextLabel::get_window_pos_x() const
{
	float offset = 0;
	if(this->parent != nullptr && !this->parent->is_window())
		offset = dynamic_cast<Panel*>(this->parent)->get_x();
	return offset + (this->x + this->width);
}

float TextLabel::get_window_pos_y() const
{
	float offset = 0;
	if(this->parent != nullptr && !this->parent->is_window())
		offset = dynamic_cast<Panel*>(this->parent)->get_y();
	return offset + (this->y + this->height);
}

bool TextLabel::has_background_colour() const
{
	return this->background_colour.has_value();
}

bool TextLabel::has_text_border_colour() const
{
	return this->text_border_colour.has_value();
}

const Font& TextLabel::get_font() const
{
	return this->font;
}

void TextLabel::set_font(Font font)
{
	this->font = std::move(font);
}

const std::string& TextLabel::get_text() const
{
	return this->text;
}
void TextLabel::set_text(const std::string& new_text)
{
	this->text = new_text;
	// remember texture assignment operator is a move-assignment, so no memory droplet is created
	this->text_texture = Texture(this->font, this->text, SDL_Color({static_cast<unsigned char>(this->colour.get_x() * 255), static_cast<unsigned char>(this->colour.get_y() * 255), static_cast<unsigned char>(this->colour.get_z() * 255), static_cast<unsigned char>(255)}));
	this->width = text_texture.get_width();
	this->height = text_texture.get_height();
}

const Texture& TextLabel::get_texture() const
{
	return this->text_texture;
}

void TextLabel::set_texture(Texture texture)
{
	this->text_texture = std::move(texture);
}

GLuint TextLabel::get_background_colour_uniform() const
{
	return this->background_colour_uniform;
}

GLuint TextLabel::get_has_background_colour_uniform() const
{
	return this->has_background_colour_uniform;
}

GLuint TextLabel::get_has_text_border_colour_uniform() const
{
	return this->has_text_border_colour_uniform;
}

Button::Button(float x, float y, Vector4F colour, std::optional<Vector4F> background_colour, std::optional<Vector3F> text_border_colour, Font font, const std::string& text, const Shader& shader, MouseListener& mouse_listener): TextLabel(x, y, colour, background_colour, text_border_colour, font, text, shader), mouse_listener(mouse_listener), just_clicked(false), just_moused_over(false), on_mouse_over(nullptr), on_mouse_click(nullptr){}

void Button::update()
{
	if(this->clicked_on() && this->on_mouse_click != nullptr && !this->just_clicked && this->has_window_parent())
	{
		// if clicked on properly, run the mouse_click command, set it as just clicked and make it the focus of the window ancestor
		this->find_window_parent()->set_focused_child(this);
		this->on_mouse_click->operator()({});
		this->just_clicked = true;
	}
	else if(!this->clicked_on())
		this->just_clicked = false;
	// if click mouse button is down but this is not moused over, make sure its not focused
	if(this->mouse_listener.is_left_clicked() && !this->moused_over() && this->focused())
		this->find_window_parent()->set_focused_child(nullptr);
	TextLabel::update();
}

bool Button::focused() const
{
	if(!this->has_window_parent())
		return false;
	return this->find_window_parent()->get_focused_child() == this;
}

bool Button::is_mouse_sensitive() const
{
	return true;
}

Command* Button::get_on_mouse_over() const
{
	return this->on_mouse_over;
}

Command* Button::get_on_mouse_click() const
{
	return this->on_mouse_click;
}

void Button::set_on_mouse_over(Command* cmd)
{
	this->on_mouse_over = cmd;
}

void Button::set_on_mouse_click(Command* cmd)
{
	this->on_mouse_click = cmd;
}

bool Button::moused_over() const
{
	Vector2F mouse_pos = this->mouse_listener.get_mouse_pos();
	bool x_aligned = mouse_pos.get_x() >= (this->get_window_pos_x() - this->width) && mouse_pos.get_x() <= (this->get_window_pos_x() + this->width);
	bool y_aligned = mouse_pos.get_y() >= (this->find_window_parent()->get_height() - this->get_window_pos_y() - this->height) && mouse_pos.get_y() <= ((this->find_window_parent()->get_height() - this->get_window_pos_y() + this->height));
	return x_aligned && y_aligned;
}

bool Button::clicked_on() const
{
	// need to take into account the location where the left click was pressed to prevent dragging from firing off the button.
	Vector2F mouse_pos = this->mouse_listener.get_left_click_location();
	bool x_aligned = mouse_pos.get_x() >= (this->get_window_pos_x() - this->width) && mouse_pos.get_x() <= (this->get_window_pos_x() + this->width);
	bool y_aligned = mouse_pos.get_y() >= (this->find_window_parent()->get_height() - this->get_window_pos_y() - this->height) && mouse_pos.get_y() <= ((this->find_window_parent()->get_height() - this->get_window_pos_y() + this->height));
	return this->mouse_listener.is_left_clicked() && x_aligned && y_aligned;
}