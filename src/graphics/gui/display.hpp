#ifndef TOPAZ_GUI_DISPLAY_HPP
#define TOPAZ_GUI_DISPLAY_HPP
#include "graphics/gui/gui.hpp"
#include "graphics/texture.hpp"

class Panel : public GUI
{
public:
    Panel(Vector2I position_local_pixel_space, Vector2I dimensions_local_pixel_space, std::variant<Vector4F, const Texture*> background, GUI* parent = nullptr, std::initializer_list<GUI*> children = {});
    virtual void render(Shader& shader, int window_width_pixels, int window_height_pixels) const override;
    bool has_colour() const;
    void set_colour(Vector4F colour);
    bool has_texture() const;
    void set_texture(const Texture* texture);
    std::optional<Vector4F> get_colour() const;
    const Texture* get_texture() const;
private:
    std::variant<Vector4F, const Texture*> background;
    Mesh mesh;
};

class Label : public GUI
{
public:
    Label(Vector2I position_local_pixel_space, Font font, Vector3F text_colour, std::string text, std::optional<Vector3F> highlight_colour = {}, GUI* parent = nullptr, std::initializer_list<GUI*> children = {});
    virtual void render(Shader& shader, int window_width_pixels, int window_height_pixels) const override;
    const std::string& get_text() const;
    void set_text(std::string text);
    const Vector3F& get_text_colour() const;
    void set_text_colour(Vector3F text_colour);
    bool has_highlight() const;
    const std::optional<Vector3F>& get_highlight_colour() const;
    void set_highlight_colour(Vector3F highlight_colour);
    void remove_highlight();
    const Font& get_font() const;
    void set_font(Font font);
private:
    void update_texture();
    Font font;
    std::string text;
    Vector3F text_colour;
    std::optional<Vector3F> highlight_colour;
    std::unique_ptr<Texture> text_render_texture;
    Mesh mesh;
};

namespace tz::gui::theme::progress_bar
{
    Vector3F zero_to_twenty_five();
    Vector3F twenty_five_to_fifty();
    Vector3F fifty_to_hundred();
    Vector3F default_background_colour();
}

class ProgressBarTheme
{
public:
    using mapping_type = std::map<float, Vector3F>;
    ProgressBarTheme(mapping_type mapping = {{0.25f, tz::gui::theme::progress_bar::zero_to_twenty_five()}, {0.5f, tz::gui::theme::progress_bar::twenty_five_to_fifty()}, {1.0f, tz::gui::theme::progress_bar::fifty_to_hundred()}}, Vector3F background_colour = tz::gui::theme::progress_bar::default_background_colour());
    const Vector3F& get_colour(float progress) const;
    const Vector3F& get_background_colour() const;
private:
    mapping_type progress_to_colour;
    Vector3F background_colour;
};

class ProgressBar : public GUI
{
public:
    ProgressBar(Vector2I position_local_pixel_space, Vector2I dimensions_local_pixel_space, ProgressBarTheme theme = {}, float progress = 0.0f, GUI* parent = nullptr, std::initializer_list<GUI*> children = {});
    float get_progress() const;
    void set_progress(float progress);
private:
    ProgressBarTheme theme;
    float progress;
    Panel background, progress_bar;
};

#endif //TOPAZ_GUI_DISPLAY_HPP
