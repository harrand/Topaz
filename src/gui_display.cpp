#include "gui_display.hpp"

Panel::Panel(Vector2<int> position_local_pixel_space, Vector2<int> dimensions_local_pixel_space, std::variant<Vector4F, Texture*> background, GUI* parent, std::initializer_list<GUI*> children): GUI(position_local_pixel_space, dimensions_local_pixel_space, parent, children), background(background){}

void Panel::render(Shader& shader, int window_width_pixels, int window_height_pixels) const
{
    shader.bind();
    //shader.set_uniform<Matrix4x4>("m", tz::transform::model(Vector3F(this->position_local_pixel_space.x, this->position_local_pixel_space.y, 0.0f), {}, Vector3F(this->dimensions_local_pixel_space.x, this->dimensions_local_pixel_space.y, 1.0f)));
    //shader.set_uniform<Matrix4x4>("v", tz::transform::view(camera.position, camera.rotation));
    //shader.set_uniform<Matrix4x4>("p", camera.projection(window_width_pixels, window_height_pixels));
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

bool Panel::has_texture() const
{
    return std::holds_alternative<Texture*>(this->background);
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