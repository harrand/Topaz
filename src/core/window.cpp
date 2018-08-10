#include "core/window.hpp"
#include "graphics/texture.hpp"

bool tz::graphics::initialised = false;
bool tz::graphics::has_context = false;

Window::Window(std::string title, int x_pixels, int y_pixels, int width_pixels, int height_pixels): Window(title, {x_pixels, y_pixels}, {width_pixels, height_pixels}){}

Window::Window(std::string title, const Vector2I& position_pixel_space, const Vector2I& dimensions_pixel_space): registered_listeners{}, title(title), position_pixel_space(tz::util::gui::clamp_pixel_screen_space(position_pixel_space)), dimensions_pixel_space(tz::util::gui::clamp_pixel_screen_space(dimensions_pixel_space)), sdl_window(nullptr), close_requested(false), window_gui_element({0, 0}, {this->get_width(), this->get_height()}, nullptr, {})
{
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

    this->sdl_window = SDL_CreateWindow((this->title).c_str(), this->position_pixel_space.x, this->position_pixel_space.y, this->dimensions_pixel_space.x, this->dimensions_pixel_space.y, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    this->sdl_gl_context = SDL_GL_CreateContext(this->sdl_window);
    this->set_swap_interval_type(Window::SwapIntervalType::IMMEDIATE_UPDATES);
    tz::graphics::has_context = true;
    if(!tz::graphics::initialised)
        tz::graphics::initialise();
    //this->add_child(&this->window_gui_element);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_DEPTH_CLAMP);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

Window::~Window()
{
    for(auto listener : registered_listeners)
        this->deregister_listener(*listener);
    SDL_GL_DeleteContext(this->sdl_gl_context);
    SDL_DestroyWindow(this->sdl_window);
}

void Window::update(Shader& gui_shader, Shader* hdr_gui_shader)
{
    tz::graphics::gui_render_mode();
    this->window_gui_element.dimensions_local_pixel_space = this->dimensions_pixel_space;
    auto children = this->get_children();
    for(auto i = children.crbegin(); i != children.crend(); i++)
    {
        auto child = *i;
        if (child->uses_hdr && hdr_gui_shader != nullptr)
            child->render(*hdr_gui_shader, this->get_width(), this->get_height());
        else
            child->render(gui_shader, this->get_width(), this->get_height());
        child->update();
    }
    /*
    for(GUI* child : this->get_children())
    {
        if(child->uses_hdr && hdr_gui_shader != nullptr)
            child->render(*hdr_gui_shader, this->get_width(), this->get_height());
        else
            child->render(gui_shader, this->get_width(), this->get_height());
        child->update();
    }
     */
    SDL_GL_SwapWindow(this->sdl_window);
    SDL_Event evt;
    while(SDL_PollEvent(&evt))
    {
        for(auto listener : this->registered_listeners)
            listener->handle_events(evt);
        if(evt.type == SDL_QUIT)
            this->close_requested = true;
        if(evt.type == SDL_WINDOWEVENT)
        {
            if (evt.window.event == SDL_WINDOWEVENT_RESIZED || evt.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                SDL_GL_GetDrawableSize(this->sdl_window, &(this->dimensions_pixel_space.x), &(this->dimensions_pixel_space.y));
                //update the glViewport, so that OpenGL knows the new window size
                glViewport(0, 0, this->get_width(), this->get_height());
            }
        }
    }
    tz::graphics::scene_render_mode();
}

std::variant<Vector2I, Vector2F> Window::get_position(tz::gui::ScreenSpace screen_space) const
{
    //return screen_space == tz::gui::ScreenSpace::PIXELS ? this->position_pixel_space : tz::util::gui::to_normalised_screen_space(this->position_pixel_space);
    using namespace tz::gui;
    switch(screen_space)
    {
        case ScreenSpace::PIXELS:
            return this->position_pixel_space;
        case ScreenSpace::NORMALISED:
            return tz::util::gui::to_normalised_screen_space(this->position_pixel_space);
    }
    return this->position_pixel_space;
}

Vector2I Window::get_position_pixels() const
{
    return this->position_pixel_space;
}

void Window::set_position(std::variant<Vector2I, Vector2F> position, tz::gui::ScreenSpace screen_space)
{
    if(screen_space == tz::gui::ScreenSpace::NORMALISED && std::holds_alternative<Vector2F>(position))
        this->position_pixel_space = tz::util::gui::to_pixel_screen_space(std::get<Vector2F>(position));
    else if(screen_space == tz::gui::ScreenSpace::PIXELS && std::holds_alternative<Vector2I>(position))
        this->position_pixel_space = std::get<Vector2I>(position);
    SDL_SetWindowPosition(this->sdl_window, this->position_pixel_space.x, this->position_pixel_space.y);
}

void Window::set_position_pixels(Vector2I position_pixels)
{
    this->position_pixel_space = position_pixels;
}

void Window::centre_position(const Vector2<bool>& mask)
{
    SDL_SetWindowPosition(this->sdl_window, mask.x ? SDL_WINDOWPOS_CENTERED : this->position_pixel_space.x, mask.y ? SDL_WINDOWPOS_CENTERED : this->position_pixel_space.y);
}

std::variant<Vector2I, Vector2F> Window::get_dimensions(tz::gui::ScreenSpace screen_space) const
{
    //return screen_space == tz::gui::ScreenSpace::PIXELS ? this->dimensions_pixel_space : tz::util::gui::to_normalised_screen_space(this->dimensions_pixel_space);
    using namespace tz::gui;
    switch(screen_space)
    {
        case ScreenSpace::PIXELS:
            return this->dimensions_pixel_space;
        case ScreenSpace::NORMALISED:
            return tz::util::gui::to_normalised_screen_space(this->dimensions_pixel_space);
    }
    return this->dimensions_pixel_space;
}

Vector2I Window::get_dimensions_pixels() const
{
    return this->dimensions_pixel_space;
}

int Window::get_width() const
{
    return this->dimensions_pixel_space.x;
}

int Window::get_height() const
{
    return this->dimensions_pixel_space.y;
}

bool Window::is_close_requested() const
{
    return this->close_requested;
}

// needle

Window::SwapIntervalType Window::get_swap_interval_type() const
{
    return static_cast<SwapIntervalType>(SDL_GL_GetSwapInterval());
}

void Window::set_swap_interval_type(Window::SwapIntervalType type) const
{
    SDL_GL_SetSwapInterval(static_cast<int>(type));
}

std::string Window::get_title() const
{
    return this->title;
}

void Window::set_title(const std::string& new_title)
{
    this->title = new_title;
    SDL_SetWindowTitle(this->sdl_window, new_title.c_str());
}

bool Window::is_fullscreen() const
{
    switch(this->get_fullscreen())
    {
        case Window::FullscreenType::VIDEO_MODE:
        case Window::FullscreenType::DESKTOP_MODE:
            return true;
        default:
            return false;
    }
}

Window::FullscreenType Window::get_fullscreen() const
{
    Uint32 flags = SDL_GetWindowFlags(this->sdl_window);
    if(flags & SDL_WINDOW_FULLSCREEN)
        return Window::FullscreenType::VIDEO_MODE;
    else if(flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
        return Window::FullscreenType::DESKTOP_MODE;
    else
        return Window::FullscreenType::WINDOWED_MODE;
}

void Window::set_fullscreen(Window::FullscreenType type) const
{
    SDL_SetWindowFullscreen(this->sdl_window, static_cast<Uint32>(type));
}

bool Window::is_minimised() const
{
    return static_cast<bool>(SDL_GetWindowFlags(this->sdl_window) & SDL_WINDOW_MINIMIZED);
}

void Window::set_minimised(bool minimised)
{
    if(minimised)
        SDL_MinimizeWindow(this->sdl_window);
    else
        SDL_MaximizeWindow(this->sdl_window);
}

bool Window::has_border() const
{
    return !static_cast<bool>(SDL_GetWindowFlags(this->sdl_window) & SDL_WINDOW_BORDERLESS);
}

void Window::set_has_border(bool has_border)
{
    SDL_SetWindowBordered(this->sdl_window, has_border ? SDL_TRUE : SDL_FALSE);
}

bool Window::focused() const
{
    return static_cast<bool>(SDL_GetWindowFlags(this->sdl_window) & SDL_WINDOW_INPUT_FOCUS);
}

void Window::focus() const
{
    SDL_RaiseWindow(this->sdl_window);
}

bool Window::mouse_inside() const
{
    return static_cast<bool>(SDL_GetWindowFlags(this->sdl_window) & SDL_WINDOW_MOUSE_FOCUS);
}

bool Window::mouse_trapped() const
{
    return static_cast<bool>(SDL_GetWindowFlags(this->sdl_window) & SDL_WINDOW_INPUT_GRABBED);
}

void Window::set_render_target() const
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glViewport(0, 0, this->dimensions_pixel_space.x, this->dimensions_pixel_space.y);
}

void Window::clear(tz::graphics::BufferBit buffer_bit, float r, float g, float b, float a) const
{
    glClearColor(r, g, b, a);
    glClear(static_cast<GLbitfield>(buffer_bit));
}

void Window::register_listener(Listener& l)
{
    this->registered_listeners.push_back(&l);
    l.window = this;
}

void Window::deregister_listener(Listener& l)
{
    this->registered_listeners.erase(std::remove(this->registered_listeners.begin(), this->registered_listeners.end(), &l), this->registered_listeners.end());
}

std::vector<GUI*> Window::get_children() const
{
    std::vector<GUI*> set = this->window_gui_element.children;
    for(const auto& heap_child : this->window_gui_element.heap_children)
        set.push_back(heap_child.get());
    return set;
}

bool Window::add_child(GUI* gui)
{
    if(gui == nullptr)
        return false;
    this->window_gui_element.children.push_back(gui);
    if(gui != &this->window_gui_element)
        gui->parent = &this->window_gui_element;
    else
        gui->parent = nullptr;
    return std::find(this->window_gui_element.children.begin(), this->window_gui_element.children.end(), gui) != this->window_gui_element.children.end();
}

MessageBox::MessageBox(tz::gui::MessageBoxType type, std::string title, std::string message, Window* parent): type(type), title(title), message(message), parent(parent){}

bool MessageBox::display() const
{
    return SDL_ShowSimpleMessageBox(static_cast<Uint32>(this->type), this->title.c_str(), this->message.c_str(), this->parent != nullptr ? this->parent->sdl_window : nullptr) == 0;
}