#include "gui.hpp"
#include "SDL_mixer.h"
#include "graphics.hpp"
#include "boundary.hpp"
#include <stack>

GUI::GUI(float x, float y, float width, float height, std::optional<std::reference_wrapper<const Shader>> shader): x(x), y(y), width(width), height(height), shader(shader), parent(nullptr), children(std::deque<GUI*>()), hidden(false), use_proportional_positioning(false){}

void GUI::update() // updates all children
{
	for(GUI* child : this->children)
		child->update();
}

void GUI::destroy() // destroys all children
{
	for(GUI* child : this->children)
		child->destroy();
}

float GUI::get_window_pos_x() const
{
	float offset = 0;
	if(this->parent != nullptr)
		offset = this->parent->get_window_pos_x(); // recursive. offset = total offsets of all ancestors to get true window pos
	float x_total = (this->x + this->width);
	if(this->is_using_proportional_positioning())
		x_total = (tz::ui::create_orthographic_gui_matrix(this).inverse() * Vector4F(x_total, 0, 0, 1)).x;
	return offset + x_total;
}

float GUI::get_window_pos_y() const
{
	float offset = 0;
	if(this->parent != nullptr)
		offset = this->parent->get_window_pos_y(); // recursive. offset = total offsets of all ancestors to get true window pos
	float y_total = (this->y + this->height);
	if(this->is_using_proportional_positioning())
		y_total = (tz::ui::create_orthographic_gui_matrix(this).inverse() * Vector4F(0, y_total, 0, 1)).y;
	return offset + y_total;
}

float GUI::get_x() const
{
	return this->x;
}

float GUI::get_y() const
{
	return this->y;
}

float GUI::get_width() const
{
	return this->width;
}

float GUI::get_height() const
{
	return this->height;
}

void GUI::set_x(float x)
{
	this->x = x;
}

void GUI::set_y(float y)
{
	this->y = y;
}

void GUI::set_width(float width)
{
	this->width = width;
}

void GUI::set_height(float height)
{
	this->height = height;
}

Window* GUI::find_window_parent() const
{
	// returns a pointer to the first ancestor which is a window. if there is no such ancestor, return nullptr
	GUI* res = this->get_parent();
	while(res != nullptr && !res->is_window())
		res = res->get_parent();
	return dynamic_cast<Window*>(res);
}

bool GUI::has_window_parent() const
{
	return this->find_window_parent() != nullptr;
}

const std::optional<std::reference_wrapper<const Shader>> GUI::get_shader() const
{
	return this->shader;
}

bool GUI::has_shader() const
{
	return this->shader.has_value();
}

GUI* GUI::get_parent() const
{
	return this->parent;
}

void GUI::set_parent(GUI* parent)
{
	this->parent = parent;
}

const std::deque<GUI*>& GUI::get_children() const
{
	return this->children;
}

void GUI::add_child(GUI* child)
{
	this->children.push_back(child);
	child->set_parent(this);
}

void GUI::remove_child(GUI* child)
{
	this->children.erase(std::remove(this->children.begin(), this->children.end(), child), this->children.end());
	child->set_parent(nullptr);
}

bool GUI::is_hidden() const
{
	return this->hidden;
}

void GUI::set_hidden(bool hidden)
{
	this->hidden = hidden;
}

void GUI::set_using_proportional_positioning(bool use_proportional_positioning)
{
	this->use_proportional_positioning = use_proportional_positioning;
}

bool GUI::is_using_proportional_positioning() const
{
	return this->use_proportional_positioning;
}

bool GUI::covered() const
{
	if(!this->has_window_parent())
		return true;
	Vector2F this_minimum(this->get_window_pos_x(), this->get_window_pos_y());
	Vector2F this_maximum = this_minimum + Vector2F(this->get_width(), this->get_height());
	for(GUI* descendant : tz::ui::descendants(this->find_window_parent(), true))
	{
		if(descendant == this || descendant->is_hidden())
			continue;
		Vector2F descendant_minimum(descendant->get_window_pos_x(), descendant->get_window_pos_y());
		Vector2F descendant_maximum = descendant_minimum + Vector2F(descendant->get_width() + descendant->get_height());
		if(this_minimum > descendant_minimum && this_maximum < descendant_maximum)
			return true;
	}
	return false;
}

bool tz::graphics::initialised = false;
bool tz::graphics::has_context = false;

Window::Window(int w, int h, std::string title): GUI(0, 0, w, h, {}), title(std::move(title)), close_requested(false), focused_child(nullptr)
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
	
	this->sdl_window_pointer = SDL_CreateWindow((this->title).c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, this->get_width(), this->get_height(), SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
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

Window::Window(const Window& copy): Window(copy.get_width(), copy.get_height(), copy.title){}

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
	GUI::update();
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
				int w, h;
				SDL_GL_GetDrawableSize(this->sdl_window_pointer, &(w), &(h));
				this->set_width(w);
				this->set_height(h);
				//update the glViewport, so that OpenGL knows the new window size
				glViewport(0, 0, this->get_width(), this->get_height());
			}
		}
	}
}

void Window::destroy()
{
	GUI::destroy();
	this->close_requested = true;
}

bool Window::focused() const
{
	return SDL_GetWindowFlags(this->sdl_window_pointer) | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS;
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
	glViewport(0, 0, this->get_width(), this->get_height());
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

GUI* Window::get_focused_child() const
{
	return this->focused_child;
}

void Window::set_focused_child(GUI* child)
{
	this->focused_child = child;
}

namespace tz::ui
{
	// O(n log n) even though depth-first-traversal is normally O(n) but set insert is O(log n) so yeah
	std::set<GUI*> descendants(const GUI* gui, bool visible_only)
	{
		// depth-first search through gui child tree.
		if(gui == nullptr)
			return std::set<GUI*>();
		std::stack<const GUI*> guis;
		std::set<GUI*> descendants;
		const GUI* top;
		const GUI* end;
		// std::prev is O(n) on bidirectional iterators (which set::end() is). so this clause is O(n) where n = total number of children
		GUI* final_child = *std::prev(gui->get_children().end(), 1);
		while(final_child->get_children().size() > 0)
			final_child = *std::prev(final_child->get_children().end(), 1);
		end = final_child;
		guis.push(gui);
		while(!guis.empty())
		{
			top = guis.top();
			guis.pop();
			if(top->is_hidden() && visible_only)
				continue;
			for(GUI* child : top->get_children())
			{
				guis.push(child); // O(1)
				descendants.insert(const_cast<GUI*>(child)); // O(log n)
				if(child == end)
					break;
			}
			// so this clause is O(n log n)
		}
		return descendants;
	}
	
	// still O(n log n) (assuming range based for loop optimises to not recompute every tz::ui::descendants() (otherwise would go to O(n log^2 n) which is horrific))
	std::set<GUI*> youngest_descendants(const GUI* gui)
	{
		std::set<GUI*> youngs;
		for(GUI* descendant : tz::ui::descendants(gui))
			if(descendant->get_children().size() == 0) // add to youngs if the descendant has no children
				youngs.insert(descendant);
		return youngs;
	}
	
	Matrix4x4 create_orthographic_gui_matrix(const GUI* gui)
	{
		if(gui == nullptr || !gui->has_window_parent())
			return Matrix4x4::identity();
		return Matrix4x4::create_orthographic_matrix(gui->find_window_parent()->get_width(), 0.0f, gui->find_window_parent()->get_height(), 0.0f, -1.0f, 1.0f);
	}
}