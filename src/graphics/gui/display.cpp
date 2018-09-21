#include "graphics/gui/display.hpp"

Panel::Panel(Vector2I position_local_pixel_space, Vector2I dimensions_local_pixel_space, std::variant<Vector4F, const Texture*> background, GUI* parent, std::initializer_list<GUI*> children): GUI(position_local_pixel_space, dimensions_local_pixel_space, parent, children), background(background), mesh(tz::util::gui::gui_quad()){}

void Panel::render(Shader& shader, int window_width_pixels, int window_height_pixels) const
{
    if(!this->is_visible())
        return;
    shader.bind();
    shader.set_uniform<bool>("has_texture", this->has_texture());
    shader.set_uniform<bool>("has_background_colour", this->has_colour());
    Matrix4x4 projection = tz::transform::orthographic_projection(window_width_pixels, 0.0f, window_height_pixels, 0.0f, -1.0f, 1.0f);
    Matrix4x4 model = projection * tz::transform::model(Vector3F(this->get_x(), this->get_y(), 0.0f), Vector3F(), Vector3F(this->get_width(), this->get_height(), 0.0f));
    shader.set_uniform<Matrix4x4>("model_matrix", model);
    if(this->has_colour())
        shader.set_uniform<Vector4F>("colour", this->get_colour().value());
    else if(this->has_texture())
    {
        this->get_texture()->bind(&shader, 0);
    }
    shader.update();
    this->mesh.render(false);
    GUI::render(shader, window_width_pixels, window_height_pixels);
}

bool Panel::has_colour() const
{
    return std::holds_alternative<Vector4F>(this->background);
}

void Panel::set_colour(Vector4F colour)
{
    this->background = colour;
}

bool Panel::has_texture() const
{
    return std::holds_alternative<const Texture*>(this->background);
}

void Panel::set_texture(const Texture* texture)
{
    this->background = texture;
    this->uses_hdr = texture->get_texture_component() == tz::graphics::TextureComponent::HDR_COLOUR_TEXTURE;
}

std::optional<Vector4F> Panel::get_colour() const
{
    if(this->has_colour())
        return std::get<Vector4F>(this->background);
    else
        return {};
}

const Texture* Panel::get_texture() const
{
    if(std::holds_alternative<const Texture*>(this->background))
        return std::get<const Texture*>(this->background);
    else
        return nullptr;
}

Label::Label(Vector2I position_local_pixel_space, Font font, Vector3F text_colour, std::string text, std::optional<Vector3F> highlight_colour, GUI* parent, std::initializer_list<GUI*> children): GUI(position_local_pixel_space, {}, parent, children), font(font), text(text), text_colour(text_colour), highlight_colour(highlight_colour), mesh(tz::util::gui::gui_quad())
{
    this->set_text(this->text);
}

void Label::render(Shader& shader, int window_width_pixels, int window_height_pixels) const
{
    if(!this->is_visible())
        return;
    shader.bind();
    shader.set_uniform<bool>("has_texture", true);
    shader.set_uniform<bool>("has_background_colour", true);
    Vector3F background_rgb = this->highlight_colour.value_or(Vector3F{});
    shader.set_uniform<Vector4F>("background_colour", {background_rgb, this->highlight_colour.has_value() ? 1.0f : 0.0f});
    Matrix4x4 projection = tz::transform::orthographic_projection(window_width_pixels, 0.0f, window_height_pixels, 0.0f, -1.0f, 1.0f);
    using namespace tz::utility::numeric;
    Matrix4x4 model = projection * tz::transform::model(Vector3F(this->get_x(), this->get_y() + (this->get_height() * 0.875f), 0.0f), Vector3F(0, consts::pi, consts::pi), Vector3F(this->get_width(), this->get_height(), 0.0f));
    shader.set_uniform<Matrix4x4>("model_matrix", model);
    shader.set_uniform<Vector4F>("colour", {this->text_colour, 1.0f});
    this->text_render_texture->bind(&shader, 0);
    shader.update();
    this->mesh.render(false);
    GUI::render(shader, window_width_pixels, window_height_pixels);
}

const std::string& Label::get_text() const
{
    return this->text;
}

void Label::set_text(std::string text)
{
    this->text = text;
    this->update_texture();
}

const Vector3F& Label::get_text_colour() const
{
    return this->text_colour;
}

void Label::set_text_colour(Vector3F text_colour)
{
    this->text_colour = text_colour;
    this->update_texture();
}

bool Label::has_highlight() const
{
    return this->highlight_colour.has_value();
}

const std::optional<Vector3F>& Label::get_highlight_colour() const
{
    return this->highlight_colour;
}

void Label::set_highlight_colour(Vector3F highlight_colour)
{
    this->highlight_colour = highlight_colour;
}

void Label::remove_highlight()
{
    this->highlight_colour = std::nullopt;
}

const Font& Label::get_font() const
{
    return this->font;
}

void Label::set_font(Font font)
{
    this->font = font;
    this->update_texture();
}

void Label::update_texture()
{
    this->text_render_texture = std::make_unique<Texture>(this->font, this->text, SDL_Color({static_cast<unsigned char>(this->text_colour.x * 255), static_cast<unsigned char>(this->text_colour.y * 255), static_cast<unsigned char>(this->text_colour.z * 255), static_cast<unsigned char>(255)}));
    this->dimensions_local_pixel_space = {this->text_render_texture->get_width(), this->text_render_texture->get_height()};
}

namespace tz::gui::theme::progress_bar
{
    Vector3F zero_to_twenty_five(){return {1.0f, 0.0f, 0.0f};}
    Vector3F twenty_five_to_fifty(){return {1.0f, 1.0f, 0.0f};}
    Vector3F fifty_to_hundred(){return {0.0f, 1.0f, 0.0f};}
    Vector3F default_background_colour(){return {0.3f, 0.3f, 0.3f};}
}

ProgressBarTheme::ProgressBarTheme(ProgressBarTheme::mapping_type mapping, Vector3F background_colour): progress_to_colour(mapping), background_colour(background_colour)
{
    if(this->progress_to_colour.empty())
        this->progress_to_colour = ProgressBarTheme{}.progress_to_colour;
}

const Vector3F& ProgressBarTheme::get_colour(float progress) const
{
    return this->progress_to_colour.lower_bound(progress)->second;
}

const Vector3F& ProgressBarTheme::get_background_colour() const
{
    return this->background_colour;
}

ProgressBar::ProgressBar(Vector2I position_local_pixel_space, Vector2I dimensions_local_pixel_space, ProgressBarTheme theme, float progress, GUI* parent, std::initializer_list<GUI*> children): GUI(position_local_pixel_space, dimensions_local_pixel_space, parent, children), theme(theme), progress(progress), background({}, dimensions_local_pixel_space, Vector4F{this->theme.get_background_colour(), 1.0f}, this), progress_bar({5, 5}, {}, Vector4F{1.0f, 0.0f, 0.0f, 1.0f}, &this->background)
{
    this->add_child(&this->background);
    this->background.set_local_dimensions_normalised_space({1.0f, 1.0f});
    this->background.add_child(&this->progress_bar);
    this->progress_bar.set_local_position_normalised_space({0.0f, 0.05f});
    int pixels = this->progress_bar.get_local_position_pixel_space().y;
    this->progress_bar.set_local_position_pixel_space({pixels, pixels});
    // set local proportional dimensions to be equal to progress percentage.
    this->set_progress(progress);
}

float ProgressBar::get_progress() const
{
    return this->progress;
}

void ProgressBar::set_progress(float progress)
{
    this->progress = std::clamp(progress, 0.0f, 1.0f);
    this->progress_bar.set_local_dimensions_normalised_space({this->progress * 0.95f, 0.90f});
    // Sort out colour matching.
    this->progress_bar.set_colour({this->theme.get_colour(this->progress), 1.0f});
    /*
    if(this->progress > 0.5f)
        this->progress_bar.set_colour({0.0f, 1.0f, 0.0f, 1.0f});
    else if(this->progress > 0.25f)
        this->progress_bar.set_colour({1.0f, 1.0f, 0.0f, 1.0f});
    else
        this->progress_bar.set_colour({1.0f, 0.0f, 0.0f, 1.0f});
    */
}