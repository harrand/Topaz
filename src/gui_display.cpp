#include "gui_display.hpp"

Panel::Panel(float x, float y, float width, float height, Vector4F colour, const Shader& shader): GUI(x, y, width, height, shader), colour(colour), quad(tz::graphics::create_quad()), colour_uniform(glGetUniformLocation(this->shader.value().get().get_program_handle(), "colour")), model_matrix_uniform(glGetUniformLocation(this->shader.value().get().get_program_handle(), "model_matrix")){}

const Vector4F& Panel::get_colour() const
{
	return this->colour;
}

void Panel::set_colour(Vector4F colour)
{
	this->colour = colour;
}

void Panel::update()
{
	if(!this->hidden)
	{
		//update uniforms & bind & render. THEN update all children (unless the panel is hidden in which case do nothing)
		this->shader.value().get().bind();
		glUniform1i(glGetUniformLocation(this->shader.value().get().get_program_handle(), "has_texture"), false);
		glUniform1i(glGetUniformLocation(this->shader.value().get().get_program_handle(), "has_background_colour"), false);
		glUniform1i(glGetUniformLocation(this->shader.value().get().get_program_handle(), "has_text_border_colour"), false);
		glUniform4f(this->colour_uniform, this->colour.get_x(), this->colour.get_y(), this->colour.get_z(), this->colour.get_w());
		Matrix4x4 projection;
		if(this->has_window_parent() && !this->use_proportional_positioning)
			projection = tz::ui::create_orthographic_gui_matrix(this);
		else
			projection = Matrix4x4::identity();
		glUniformMatrix4fv(this->model_matrix_uniform, 1, GL_TRUE, (projection * Matrix4x4::create_model_matrix(Vector3F(this->get_window_pos_x(), this->get_window_pos_y(), 0.0f), Vector3F(), Vector3F(this->width, this->height, 0.0f))).fill_data().data());
		this->quad.render(false);
		GUI::update();
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

TextLabel::TextLabel(float x, float y, Vector4F colour, std::optional<Vector4F> background_colour, std::optional<Vector3F> text_border_colour, Font font, const std::string& text, const Shader& shader): Panel(x, y, this->text_texture.get_width(), this->text_texture.get_height(), colour, shader), background_colour(background_colour), text_border_colour(text_border_colour), font(font), text(text), text_texture(this->font, this->text, SDL_Color({static_cast<unsigned char>(this->colour.get_x() * 255), static_cast<unsigned char>(this->colour.get_y() * 255), static_cast<unsigned char>(this->colour.get_z() * 255), static_cast<unsigned char>(255)})), background_colour_uniform(glGetUniformLocation(this->shader.value().get().get_program_handle(), "background_colour")), has_background_colour_uniform(glGetUniformLocation(this->shader.value().get().get_program_handle(), "has_background_colour")), text_border_colour_uniform(glGetUniformLocation(this->shader.value().get().get_program_handle(), "text_border_colour")), has_text_border_colour_uniform(glGetUniformLocation(this->shader.value().get().get_program_handle(), "has_text_border_colour"))
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
		glUniform1i(glGetUniformLocation(this->shader.value().get().get_program_handle(), "has_texture"), true);
		glUniform1i(this->has_background_colour_uniform, this->has_background_colour() ? true : false);
		glUniform1i(this->has_text_border_colour_uniform, this->has_text_border_colour() ? true : false);
		if(this->has_background_colour())
			glUniform4f(this->background_colour_uniform, this->background_colour.value().get_x(), this->background_colour.value().get_y(), this->background_colour.value().get_z(), this->background_colour.value().get_w());
		if(this->has_text_border_colour())
			glUniform3f(this->text_border_colour_uniform, this->text_border_colour.value().get_x(), this->text_border_colour.value().get_y(), this->text_border_colour.value().get_z());
		Matrix4x4 projection;
		if(this->has_window_parent() && !this->use_proportional_positioning)
			projection = tz::ui::create_orthographic_gui_matrix(this);
		else
			projection = Matrix4x4::identity();
		glUniformMatrix4fv(this->model_matrix_uniform, 1, GL_TRUE, (projection * Matrix4x4::create_model_matrix(Vector3F(this->get_window_pos_x(), this->get_window_pos_y(), 0.0f), Vector3F(), Vector3F(this->width, this->height, 0.0f))).fill_data().data());
		this->text_texture.bind(this->shader.value().get().get_program_handle(), 0);
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
	this->text_texture = Texture(this->font, this->text, SDL_Color({static_cast<unsigned char>(this->colour.get_x() * 255), static_cast<unsigned char>(this->colour.get_y() * 255), static_cast<unsigned char>(this->colour.get_z() * 255), static_cast<unsigned char>(255)}));
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

GLuint TextLabel::get_background_colour_uniform() const
{
	return this->background_colour_uniform;
}

GLuint TextLabel::get_has_background_colour_uniform() const
{
	return this->has_background_colour_uniform;
}

GLuint TextLabel::get_has_text_border_colour_uniform() const
{
	return this->has_text_border_colour_uniform;
}