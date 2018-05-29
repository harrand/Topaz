#include "gui_display.hpp"

Panel::Panel(Vector2<int> position_local_pixel_space, Vector2<int> dimensions_local_pixel_space, std::variant<Vector4F, Texture*> background, GUI* parent, std::initializer_list<GUI*> children): GUI(position_local_pixel_space, dimensions_local_pixel_space, parent, children), background(background), mesh(tz::util::gui::gui_quad()){}

void Panel::render(Shader& shader, int window_width_pixels, int window_height_pixels) const
{
    shader.bind();
    shader.set_uniform<bool>("has_texture", this->has_texture());
    shader.set_uniform<bool>("has_background_colour", this->has_colour());
    shader.set_uniform<bool>("has_text_border_colour", false);
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
    return std::holds_alternative<Texture*>(this->background);
}

void Panel::set_texture(Texture* texture)
{
    this->background = texture;
}

std::optional<Vector4F> Panel::get_colour() const
{
    if(this->has_colour())
        return std::get<Vector4F>(this->background);
    else
        return {};
}

Texture* Panel::get_texture() const
{
    if(std::holds_alternative<Texture*>(this->background))
        return std::get<Texture*>(this->background);
    else
        return nullptr;
}

Label::Label(Vector2<int> position_local_pixel_space, Font font, Vector3F text_colour, std::string text, GUI* parent, std::initializer_list<GUI*> children): GUI(position_local_pixel_space, {}, parent, children), font(font), text(text), text_colour(text_colour), mesh(tz::util::gui::gui_quad())
{
    this->set_text(this->text);
}

void Label::render(Shader& shader, int window_width_pixels, int window_height_pixels) const
{
    shader.bind();
    shader.set_uniform<bool>("has_texture", true);
    shader.set_uniform<bool>("has_background_colour", true);
    shader.set_uniform<bool>("has_text_border_colour", false);
    shader.set_uniform<Vector4F>("background_colour", {});
    Matrix4x4 projection = tz::transform::orthographic_projection(window_width_pixels, 0.0f, window_height_pixels, 0.0f, -1.0f, 1.0f);
    Matrix4x4 model = projection * tz::transform::model(Vector3F(this->get_x(), this->get_y() + this->get_height(), 0.0f), Vector3F(0, tz::consts::pi, tz::consts::pi), Vector3F(this->get_width(), this->get_height(), 0.0f));
    shader.set_uniform<Matrix4x4>("model_matrix", model);
    shader.set_uniform<Vector4F>("colour", {this->text_colour, 1.0f});
    this->text_render_texture->bind(&shader, 0);
    shader.update();
    this->mesh.render(false);
    GUI::render(shader, window_width_pixels, window_height_pixels);
}

void Label::set_text(std::string text)
{
    this->text = text;
    this->update_texture();
}

void Label::set_text_colour(Vector3F text_colour)
{
    this->text_colour = text_colour;
    this->update_texture();
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