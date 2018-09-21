#ifndef TOPAZ_GUI_HPP
#define TOPAZ_GUI_HPP
#include "graphics/mesh.hpp"
#include "graphics/shader.hpp"
#include "graphics/camera.hpp"
#include <SDL2/SDL.h>
#include <set>
#include <memory>
#include <variant>

namespace tz::gui
{
        /**
         * Consider a 800x600 pixel screen:
         * for ScreenSpace::PIXELS:
         * +------------+
         * |            |   ^
         * |            |  600
         * |            |   v
         * +------------+
         *     < 800 >
         *
         * for ScreenSpace::NORMALISED:
         * +------------+
         * |            |   ^
         * |            |   1
         * |            |   v
         * +------------+
         *     < 1 >
         */
    enum class ScreenSpace : unsigned int { PIXELS, NORMALISED };
}

class GUI
{
public:
    friend class Window;
    GUI(Vector2I position_local_pixel_space, Vector2I dimensions_local_pixel_space, GUI* parent = nullptr, std::initializer_list<GUI*> children = {}, bool hdr = false);
    virtual void update();
    virtual void destroy();
    virtual bool key_sensitive() const{return false;}
    virtual bool mouse_sensitive() const{return false;}
    virtual void render(Shader& shader, int window_width_pixels, int window_height_pixels) const;
    /**
     * Get the x-coordinate of the top-left of this GUI element on the screen.
     * @return - X-coordinate of the GUI on the screen
     */
    int get_x() const;
    /**
     * Get the y-coordinate of the top-left of this GUI element on the screen.
     * @return - Y-coordinate of the GUI on the screen
     */
    int get_y() const;
    void set_x(int x);
    void set_y(int y);
    /**
     * Get the local-position (relative to the parent, or screen if there isn't one) of the top-left of the GUI, depending on what screen-space is specified.
     * @param screen_space - The specified screen-space to format the return value as
     * @return - A Vector2I if ScreenSpace::PIXELS is passed, or a Vector2F if ScreenSpace::NORMALISED is passed
     */
    std::variant<Vector2I, Vector2F> get_local_position(tz::gui::ScreenSpace screen_space) const;
    Vector2I get_local_position_pixel_space() const;
    Vector2F get_local_position_normalised_space() const;
    std::variant<Vector2I, Vector2F> get_screen_position(tz::gui::ScreenSpace screen_space) const;
    Vector2I get_screen_position_pixel_space() const;
    Vector2F get_screen_position_normalised_space() const;
    void set_local_position_pixel_space(Vector2I position_local_pixel_space);
    void set_local_position_normalised_space(Vector2F position_local_normalised_space);
    void set_local_dimensions_pixel_space(Vector2I dimensions_local_pixel_space);
    void set_local_dimensions_normalised_space(Vector2F dimensions_local_normalised_space);
    /**
     * Get the width of this GUI, in pixels.
     * @return - Width of GUI in pixels.
     */
    int get_width() const;
    /**
     * Get the height of this GUI, in pixels.
     * @return - Height of GUI in pixels.
     */
    int get_height() const;
    /**
     * Get proportion of the screen that the width of this GUI takes up.
     * @return - Float value between 0.0 and 1.0f
     */
    float get_normalised_screen_width() const;
    /**
     * Get proportion of the screen that the height of this GUI takes up.
     * @return - Float value between 0.0 and 1.0f
     */
    float get_normalised_screen_height() const;
    bool is_visible() const;
    void set_visible(bool visible, bool affect_children = true);
    GUI* get_parent() const;
    void set_parent(GUI* new_parent);
    std::vector<GUI*> get_children() const;
    template<class GUIType, typename... Args>
    GUIType& emplace_child(Args&&... args);
    bool add_child(GUI* gui);
    GUI* get_root() const;
    std::vector<GUI*> get_descendants();
    std::vector<GUI*> get_youngest_descendants();
    template<template<typename> class Container>
    std::unordered_set<GUI*> get_occluders(const Container<GUI*>& gui_elements);
    template<template<typename> class Container>
    bool is_occluded_by(const Container<GUI*>& gui_elements);
    bool uses_hdr;
protected:
    Vector2I position_local_pixel_space, dimensions_local_pixel_space;
    GUI* parent;
    std::vector<GUI*> children;
    std::vector<std::shared_ptr<GUI>> heap_children;
    bool visible;
    std::optional<Vector2F> position_normalised_space, dimensions_normalised_space;
};

/**
 * Common gui helper functions, such as space-conversions.
 */
namespace tz::util::gui
{
    /**
     * Utility functions specific to the screen display.
     */
    namespace display
    {
        /**
         * Get the resolution of the current display.
         * @return - {w, h} in pixels
         */
        Vector2I resolution();
        /**
         * Get the refresh rate of the current display.
         * @return - Refresh-rate, in hertz (Hz)
         */
        int refresh_rate();
    }
    /**
     * Convert a normalised-screen-space position into a pixel-screen-space position.
     * @param normalised_screen_space - The position between {0, 0} and {1.0f, 1.0f}
     * @param resolution - Resolution through which to convert
     * @return - The transformed position in pixel-screen-space
     */
    Vector2I to_pixel_screen_space(const Vector2F& normalised_screen_space, const Vector2I& resolution = display::resolution());
    /**
     * Convert a pixel-screen-space position into a normalised-screen-space position.
     * @param pixel_screen_space - The pixel position, such as {800, 600}
     * @param resolution - Resolution through which to convert
     * @return - The transformed position in normalised-screen-space (between {0, 0} and {1.0f, 1.0f})
     */
    Vector2F to_normalised_screen_space(const Vector2I& pixel_screen_space, const Vector2I& resolution = display::resolution());
    /**
     * Clamp a pixel-screen-space position, to ensure that it is valid. See the following example:
     *      Example Resolution = 800x600
     *      Input = {900, 1024} (isn't on the screen)
     *      Output = {800, 600} (clamps to maximum)
     * @param pixel_screen_space
     * @param resolution
     * @return
     */
    Vector2I clamp_pixel_screen_space(const Vector2I& pixel_screen_space, const Vector2I& resolution = display::resolution());
    /**
     * Generate the expected GUI quad (where {0, 0} modelspace is the bottom-left of the quad, instead of the middle)
     * @return - OpenGL quad, in a slightly manipulated format designed to make GUI simpler.
     */
    Mesh gui_quad();
}

#include "gui.inl"

#endif //TOPAZ_GUI_HPP
