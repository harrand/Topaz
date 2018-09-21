#include "graphics/gui/gui.hpp"
#include <stack>

GUI::GUI(Vector2I position_local_pixel_space, Vector2I dimensions_local_pixel_space, GUI* parent, std::initializer_list<GUI*> children, bool hdr): uses_hdr(hdr), position_local_pixel_space(position_local_pixel_space), dimensions_local_pixel_space(dimensions_local_pixel_space), parent(parent), children(children), visible(true), position_normalised_space(std::nullopt), dimensions_normalised_space(std::nullopt){}

void GUI::update()
{
    if(this->position_normalised_space.has_value())
    {
        Vector2I resolution = this->parent != nullptr ? this->parent->dimensions_local_pixel_space : tz::util::gui::display::resolution();
        this->position_local_pixel_space = tz::util::gui::to_pixel_screen_space(this->position_normalised_space.value(), resolution);
    }
    if(this->dimensions_normalised_space.has_value())
    {
        Vector2I resolution = this->parent != nullptr ? this->parent->dimensions_local_pixel_space : tz::util::gui::display::resolution();
        this->dimensions_local_pixel_space = tz::util::gui::to_pixel_screen_space(this->dimensions_normalised_space.value(), resolution);
    }
    for(auto i = this->children.rbegin(); i != this->children.rend(); i++)
        (*i)->update();
    /*
    for(GUI* child : this->children)
        child->update();
    */
}

void GUI::destroy()
{
    bool contained_stack = std::find(this->parent->children.begin(), this->parent->children.end(), this) != this->parent->children.end();
    this->parent->children.erase(std::find(this->parent->children.begin(), this->parent->children.end(), this));
    if(contained_stack)
        return;
    for(auto& heap_child_shared : this->parent->heap_children)
    {
        GUI* heap_child = heap_child_shared.get();
        if (heap_child == this)
        {
            this->parent->heap_children.erase(std::find(this->parent->heap_children.begin(), this->parent->heap_children.end(), heap_child_shared));
            return;
        }
    }
}

void GUI::render(Shader& shader, int window_width_pixels, int window_height_pixels) const
{
    auto children = this->get_children();
    for(auto i = children.crbegin(); i != children.crend(); i++)
        (*i)->render(shader, window_width_pixels, window_height_pixels);
}

int GUI::get_x() const
{
    return this->get_screen_position_pixel_space().x;
}

int GUI::get_y() const
{
    return this->get_screen_position_pixel_space().y;
}

void GUI::set_x(int x)
{
    if(this->parent == nullptr)
        this->position_local_pixel_space.x = x;
    else
        this->position_local_pixel_space.x = x - this->parent->get_x();
}

void GUI::set_y(int y)
{
    if(this->parent == nullptr)
        this->position_local_pixel_space.y = y;
    else
        this->position_local_pixel_space.y = y - this->parent->get_y();
}

void GUI::set_local_position_pixel_space(Vector2I position_local_pixel_space)
{
    this->position_local_pixel_space = position_local_pixel_space;
    this->position_normalised_space = std::nullopt;
}

void GUI::set_local_position_normalised_space(Vector2F position_local_normalised_space)
{
    this->position_normalised_space = position_local_normalised_space;
    /*
    Vector2I resolution = this->parent != nullptr ? this->parent->dimensions_local_pixel_space : tz::util::gui::display::resolution();
    this->position_local_pixel_space = tz::util::gui::to_pixel_screen_space(position_local_normalised_space, resolution);
     */
}

void GUI::set_local_dimensions_pixel_space(Vector2I dimensions_local_pixel_space)
{
    this->dimensions_local_pixel_space = dimensions_local_pixel_space;
    this->dimensions_normalised_space = std::nullopt;
}

void GUI::set_local_dimensions_normalised_space(Vector2F dimensions_local_normalised_space)
{
    this->dimensions_normalised_space = dimensions_local_normalised_space;
    /*
    Vector2I resolution = this->parent != nullptr ? this->parent->dimensions_local_pixel_space : tz::util::gui::display::resolution();
    this->dimensions_local_pixel_space = tz::util::gui::to_pixel_screen_space(dimensions_local_normalised_space, resolution);
     */
}

int GUI::get_width() const
{
    return this->dimensions_local_pixel_space.x;
}

int GUI::get_height() const
{
    return this->dimensions_local_pixel_space.y;
}

float GUI::get_normalised_screen_width() const
{
    return tz::util::gui::to_normalised_screen_space(this->dimensions_local_pixel_space).x;
}

float GUI::get_normalised_screen_height() const
{
    return tz::util::gui::to_normalised_screen_space(this->dimensions_local_pixel_space).y;
}

bool GUI::is_visible() const
{
    return this->visible;
}

void GUI::set_visible(bool visible, bool affect_children)
{
    this->visible = visible;
    if(affect_children)
    {
        for (GUI *child : this->get_children())
            child->set_visible(visible, affect_children);
    }

}

GUI* GUI::get_parent() const
{
    return this->parent;
}

void GUI::set_parent(GUI* new_parent)
{
    GUI* old_parent = this->parent;
    this->parent = new_parent;
    old_parent->children.erase(std::find(old_parent->children.begin(), old_parent->children.end(), this));
}

std::vector<GUI*> GUI::get_children() const
{
    std::vector<GUI*> set = this->children;
    for(const auto& heap_child : this->heap_children)
        set.push_back(heap_child.get());
    return set;
}

bool GUI::add_child(GUI* gui)
{
    if(gui == nullptr)
        return false;
    this->children.push_back(gui);
    return std::find(this->children.begin(), this->children.end(), gui) != this->children.end();
}

// private helpers
std::variant<Vector2I, Vector2F> GUI::get_local_position(tz::gui::ScreenSpace screen_space) const
{
    using namespace tz::gui;
    Vector2I resolution = this->parent == nullptr ? tz::util::gui::display::resolution() : this->parent->dimensions_local_pixel_space;
    switch(screen_space)
    {
        case ScreenSpace::PIXELS:
            return this->position_local_pixel_space;
        case ScreenSpace::NORMALISED:
            return tz::util::gui::to_normalised_screen_space(this->position_local_pixel_space, resolution);
    }
    return this->position_local_pixel_space;
}

Vector2I GUI::get_local_position_pixel_space() const
{
    return std::get<Vector2I>(this->get_local_position(tz::gui::ScreenSpace::PIXELS));
}

Vector2F GUI::get_local_position_normalised_space() const
{
    return std::get<Vector2F>(this->get_local_position(tz::gui::ScreenSpace::NORMALISED));
}

std::variant<Vector2I, Vector2F> GUI::get_screen_position(tz::gui::ScreenSpace screen_space) const
{
    if(this->parent == nullptr)
        return this->get_local_position(screen_space);
    // std::variant doesn't define operator+, need to branch out
    //this->parent->get_screen_position(screen_space) + this->get_local_position(screen_space);
    switch(screen_space)
    {
        case tz::gui::ScreenSpace::PIXELS:
            return std::get<Vector2I>(this->parent->get_screen_position(screen_space)) + std::get<Vector2I>(this->get_local_position(screen_space));
        case tz::gui::ScreenSpace::NORMALISED:
            return std::get<Vector2F>(this->parent->get_screen_position(screen_space)) + std::get<Vector2F>(this->get_local_position(screen_space));
    }
    return std::get<Vector2I>(this->parent->get_screen_position(screen_space)) + std::get<Vector2I>(this->get_local_position(screen_space));
}

Vector2I GUI::get_screen_position_pixel_space() const
{
    return std::get<Vector2I>(this->get_screen_position(tz::gui::ScreenSpace::PIXELS));
}

Vector2F GUI::get_screen_position_normalised_space() const
{
    return std::get<Vector2F>(this->get_screen_position(tz::gui::ScreenSpace::NORMALISED));
}

GUI* GUI::get_root() const
{
    GUI* root = this->get_parent();
    while(root != nullptr && root->get_parent() != nullptr)
        root = root->get_parent();
    return root;
}

std::vector<GUI*> GUI::get_descendants()
{
    std::stack<GUI*> gui_stack;
    std::vector<GUI*> descendants;
    gui_stack.push(this);
    while(!gui_stack.empty())
    {
        GUI* current = gui_stack.top();
        gui_stack.pop();
        for(GUI* child : current->get_children())
        {
            gui_stack.push(child);
            descendants.push_back(child);
        }
    }
    return descendants;
}

std::vector<GUI*> GUI::get_youngest_descendants()
{
    std::vector<GUI*> youngs;
    for(GUI* descendant : this->get_descendants())
        if(descendant->get_children().empty()) // add to youngs if the descendant has no children
            youngs.push_back(descendant);
    return youngs;
}

namespace tz::util::gui
{
    namespace display
    {
        Vector2I resolution()
        {
            /// Returns dimensions of native display mode. So if the full=screen video-mode is activated, it will still return the desktop mode.
            SDL_DisplayMode display_mode;
            SDL_GetCurrentDisplayMode(0, &display_mode);
            return {display_mode.w, display_mode.h};
        }

        int refresh_rate()
        {
            /// Returns dimensions of native display mode. So if the full-screen video-mode is activated, it will still return the desktop mode.
            SDL_DisplayMode display_mode;
            SDL_GetDesktopDisplayMode(0, &display_mode);
            return display_mode.refresh_rate;
        }
    }

    Vector2I to_pixel_screen_space(const Vector2F& normalised_screen_space, const Vector2I& resolution)
    {
        return {static_cast<int>(std::round(normalised_screen_space.x * resolution.x)), static_cast<int>(std::round(normalised_screen_space.y * resolution.y))};
    }

    Vector2F to_normalised_screen_space(const Vector2I& pixel_screen_space, const Vector2I& resolution)
    {
        return {static_cast<float>(pixel_screen_space.x) / resolution.x, static_cast<float>(pixel_screen_space.y) / resolution.y};
    }

    Vector2I clamp_pixel_screen_space(const Vector2I& pixel_screen_space, const Vector2I& resolution)
    {
        return {std::clamp(pixel_screen_space.x, 0, resolution.x), std::clamp(pixel_screen_space.y, 0, resolution.y)};
    }

    Mesh gui_quad()
    {
        /*
         * tz::graphics::create_quad():
         * Create a single Quad, like so:
		 * *--------* [x + width, y + height]
		 * |		|
		 * |	*	|
		 * | [x, y] |
		 * *--------*
		 * [x - width, y - height]
         *
         * We want this format:
         * *--------* [1, 1]
		 * |		|
		 * |	*	|
		 * | [x, y] |
		 * *--------*
		 * [0, 0]
         * thus:
         * x + width/2, y + height/2 = 1
         * x - width/2, x - height/2 = 0
         * x = y = 0.5f
         * width = height = 1.0f
         */
        return tz::graphics::create_quad(0.5f, 0.5f, 0.5f, 0.5f);
    }
}
