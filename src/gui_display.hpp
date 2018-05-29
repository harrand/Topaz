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
    void set_colour(Vector4F colour);
    bool has_texture() const;
    void set_texture(Texture* texture);
    std::optional<Vector4F> get_colour() const;
    Texture* get_texture() const;
private:
    std::variant<Vector4F, Texture*> background;
};

class Label : public GUI
{
public:
    Label(Vector2<int> position_local_pixel_space, Font font, Vector3F text_colour, std::string text, GUI* parent = nullptr, std::initializer_list<GUI*> children = {});
    virtual void render(Shader& shader, int window_width_pixels, int window_height_pixels) const override;
    void set_text(std::string text);
    void set_text_colour(Vector3F text_colour);
    void set_font(Font font);
private:
    void update_texture();
    Font font;
    std::string text;
    Vector3F text_colour;
    std::unique_ptr<Texture> text_render_texture;
};

#endif //TOPAZ_GUI_DISPLAY_HPP
