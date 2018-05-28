#include "gui.hpp"

GUI::GUI(Vector2<int> position_local_pixel_space, Vector2<int> dimensions_local_pixel_space, GUI* parent, std::initializer_list<GUI*> children): position_local_pixel_space(position_local_pixel_space), dimensions_local_pixel_space(dimensions_local_pixel_space), parent(parent), children(children){}

int GUI::get_x() const
{
    return this->get_screen_position_pixel_space().x;
}

int GUI::get_y() const
{
    return this->get_screen_position_pixel_space().y;
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


GUI* GUI::get_parent() const
{
    return this->parent;
}

void GUI::set_parent(GUI* new_parent)
{
    GUI* old_parent = this->parent;
    this->parent = new_parent;
    old_parent->children.erase(this);
}

std::unordered_set<GUI*> GUI::get_children() const
{
    std::unordered_set<GUI*> set = this->children;
    for(const auto& heap_child : this->heap_children)
        set.insert(heap_child.get());
    return set;
}

bool GUI::add_child(GUI* gui)
{
    if(gui == nullptr)
        return false;
    return this->children.insert(gui).second;
}

// private helpers
std::variant<Vector2<int>, Vector2F> GUI::get_local_position(tz::gui::ScreenSpace screen_space) const
{
    using namespace tz::gui;
    Vector2<int> resolution = this->parent == nullptr ? tz::util::gui::display::resolution() : this->parent->dimensions_local_pixel_space;
    switch(screen_space)
    {
        case ScreenSpace::PIXELS:
            return this->position_local_pixel_space;
        case ScreenSpace::NORMALISED:
            return tz::util::gui::to_normalised_screen_space(this->position_local_pixel_space, resolution);
    }
}

Vector2<int> GUI::get_local_position_pixel_space() const
{
    return std::get<Vector2<int>>(this->get_local_position(tz::gui::ScreenSpace::PIXELS));
}

Vector2F GUI::get_local_position_normalised_space() const
{
    return std::get<Vector2F>(this->get_local_position(tz::gui::ScreenSpace::NORMALISED));
}

std::variant<Vector2<int>, Vector2F> GUI::get_screen_position(tz::gui::ScreenSpace screen_space) const
{
    if(this->parent == nullptr)
        return this->get_local_position(screen_space);
    // std::variant doesn't define operator+, need to branch out
    //this->parent->get_screen_position(screen_space) + this->get_local_position(screen_space);
    switch(screen_space)
    {
        case tz::gui::ScreenSpace::PIXELS:
            return std::get<Vector2<int>>(this->parent->get_screen_position(screen_space)) + std::get<Vector2<int>>(this->get_local_position(screen_space));
        case tz::gui::ScreenSpace::NORMALISED:
            return std::get<Vector2F>(this->parent->get_screen_position(screen_space)) + std::get<Vector2F>(this->get_local_position(screen_space));
    }
}

Vector2<int> GUI::get_screen_position_pixel_space() const
{
    return std::get<Vector2<int>>(this->get_screen_position(tz::gui::ScreenSpace::PIXELS));
}

Vector2F GUI::get_screen_position_normalised_space() const
{
    return std::get<Vector2F>(this->get_screen_position(tz::gui::ScreenSpace::NORMALISED));
}

namespace tz::util::gui
{
    namespace display
    {
        Vector2<int> resolution()
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

    Vector2<int> to_pixel_screen_space(const Vector2F& normalised_screen_space, const Vector2<int>& resolution)
    {
        return {static_cast<int>(std::round(normalised_screen_space.x * resolution.x)), static_cast<int>(std::round(normalised_screen_space.y * resolution.y))};
    }

    Vector2F to_normalised_screen_space(const Vector2<int>& pixel_screen_space, const Vector2<int>& resolution)
    {
        return {static_cast<float>(pixel_screen_space.x) / resolution.x, static_cast<float>(pixel_screen_space.y) / resolution.y};
    }

    Vector2<int> clamp_pixel_screen_space(const Vector2<int>& pixel_screen_space, const Vector2<int>& resolution)
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
         * x + width, y + height = 1
         * x - width, x - height = 0
         * x = y = 0.5f
         * width = height = 0.5f
         */
        return tz::graphics::create_quad(0.5f, 0.5f, 0.5f, 0.5f);
    }
}
