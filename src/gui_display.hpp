#ifndef TOPAZ_GUI_DISPLAY_HPP
#define TOPAZ_GUI_DISPLAY_HPP
#include "gui.hpp"
#include "graphics/texture.hpp"

class Panel : public GUI
{
public:
    Panel(Vector2<int> position_local_pixel_space, Vector2<int> dimensions_local_pixel_space, std::variant<Vector4F, Texture*> background, GUI* parent = nullptr, std::initializer_list<GUI*> children = {});
    virtual void render(Shader& shader, int window_width_pixels, int window_height_pixels) const override;
    bool has_colour() const;
    bool has_texture() const;
    std::optional<Vector4F> get_colour() const;
    Texture* get_texture() const;
private:
    std::variant<Vector4F, Texture*> background;
};

#endif //TOPAZ_GUI_DISPLAY_HPP
