#include "gui_display.hpp"

Panel::Panel(float x, float y, float width, float height, Vector4F colour, Shader& shader): GUI(x, y, width, height, shader), texture(nullptr), colour(colour), quad(tz::graphics::create_quad()){}

const Vector4F& Panel::get_colour() const
{
	return this->colour;
}

void Panel::set_colour(Vector4F colour)
{
	this->colour = colour;
}

const Texture* Panel::get_texture() const
{
	return this->texture;
}

void Panel::set_texture(Texture* texture)
{
	this->texture = texture;
}

void Panel::disable_texture()
{
	this->texture = nullptr;
}

bool Panel::has_texture() const
{
	return this->texture != nullptr;
}

void Panel::update()
{
	if(!this->hidden)
	{
		this->render_panel(this->colour);
	}
}

void Panel::destroy()
{
	// kill all children before killing itself (that was a dark comment)
	GUI::destroy();
	if(this->parent != nullptr)
	{
		this->parent->remove_child(this);
		this->parent = nullptr;
	}
}

void Panel::render_panel(Vector4F colour)
{
	//update uniforms & bind & render. THEN update all children (unless the panel is hidden in which case do nothing)
	this->shader.value().get().bind();
	this->shader.value().get().set_uniform<bool>("has_texture", this->has_texture());
	this->shader.value().get().set_uniform<bool>("has_background_colour", false);
	this->shader.value().get().set_uniform<bool>("has_text_border_colour", false);
	this->shader.value().get().set_uniform<Vector4F>("colour", colour);

	Matrix4x4 projection;
	if(this->has_window_parent() && !this->use_proportional_positioning)
		projection = tz::ui::create_orthographic_gui_matrix(this);
	else
		projection = Matrix4x4::identity();
	this->shader.value().get().set_uniform<Matrix4x4>("model_matrix", projection * tz::transform::model(Vector3F(this->get_window_pos_x(), this->get_window_pos_y(), 0.0f), Vector3F(), Vector3F(this->width, this->height, 0.0f)));
	if(this->has_texture())
		this->texture->bind(&(this->shader.value().get()), 0);
	this->shader.value().get().update();
	this->quad.render(false);
	GUI::update();
}

TextLabel::TextLabel(float x, float y, Vector4F colour, std::optional<Vector4F> background_colour, std::optional<Vector3F> text_border_colour, Font font, const std::string& text, Shader& shader): Panel(x, y, this->text_texture.get_width(), this->text_texture.get_height(), colour, shader), background_colour(background_colour), text_border_colour(text_border_colour), font(font), text(text), text_texture(this->font, this->text, SDL_Color({static_cast<unsigned char>(this->colour.x * 255), static_cast<unsigned char>(this->colour.y * 255), static_cast<unsigned char>(this->colour.z * 255), static_cast<unsigned char>(255)}))
{
	// Not in initialiser list because text_texture MUST be initialised after Panel, and theres no way of initialising it before without a warning so do it here.
	this->width = text_texture.get_width();
	this->height = text_texture.get_height();
}

void TextLabel::update()
{
	if(!this->hidden)
	{
		// enable blending as the glyphs from TTF fonts have alot of alpha variation which requires blending to look in any-way legible
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		this->shader.value().get().bind();
		this->shader.value().get().set_uniform<bool>("has_texture", true);
		this->shader.value().get().set_uniform<bool>("has_background_colour", this->has_background_colour());
		this->shader.value().get().set_uniform<bool>("has_text_border_colour", this->has_text_border_colour());
		if(this->has_background_colour())
			this->shader.value().get().set_uniform<Vector4F>("background_colour", this->background_colour.value());
		if(this->has_text_border_colour())
			this->shader.value().get().set_uniform<Vector3F>("text_border_colour", this->text_border_colour.value());
		
		Matrix4x4 projection;
		if(this->has_window_parent() && !this->use_proportional_positioning)
			projection = tz::ui::create_orthographic_gui_matrix(this);
		else
			projection = Matrix4x4::identity();
		this->shader.value().get().set_uniform<Matrix4x4>("model_matrix", projection * tz::transform::model(Vector3F(this->get_window_pos_x(), this->get_window_pos_y(), 0.0f), Vector3F(), Vector3F(this->width, this->height, 0.0f)));
		this->text_texture.bind(&(this->shader.value().get()), 0);
		this->shader.value().get().update();
		this->quad.render(false);
		GUI::update();
	}
}

bool TextLabel::has_background_colour() const
{
	return this->background_colour.has_value();
}

bool TextLabel::has_text_border_colour() const
{
	return this->text_border_colour.has_value();
}

const Font& TextLabel::get_font() const
{
	return this->font;
}

void TextLabel::set_font(Font font)
{
	this->font = std::move(font);
}

const std::string& TextLabel::get_text() const
{
	return this->text;
}
void TextLabel::set_text(const std::string& new_text)
{
	this->text = new_text;
	// remember texture assignment operator is a move-assignment, so no memory droplet is created
	this->text_texture = Texture(this->font, this->text, SDL_Color({static_cast<unsigned char>(this->colour.x * 255), static_cast<unsigned char>(this->colour.y * 255), static_cast<unsigned char>(this->colour.z * 255), static_cast<unsigned char>(255)}));
	this->width = text_texture.get_width();
	this->height = text_texture.get_height();
}

const Texture& TextLabel::get_texture() const
{
	return this->text_texture;
}

void TextLabel::set_texture(Texture texture)
{
	this->text_texture = std::move(texture);
}