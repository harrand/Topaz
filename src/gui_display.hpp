#ifndef GUI_DISPLAY_HPP
#define GUI_DISPLAY_HPP
#include "gui.hpp"

class Panel : public GUI
{
public:
	Panel(float x, float y, float width, float height, Vector4F colour, const Shader& shader);

	const Vector4F& get_colour() const;
	void set_colour(Vector4F colour);
	virtual void update() override;
	virtual void destroy() override;
	virtual bool focused() const override;
	virtual bool is_window() const override;
	virtual bool is_mouse_sensitive() const override;
protected:
	Vector4F colour;
	Mesh quad;
	GLuint colour_uniform, model_matrix_uniform;
};

class TextLabel : public Panel
{
public:
	TextLabel(float x, float y, Vector4F colour, std::optional<Vector4F> background_colour, std::optional<Vector3F> text_border_colour, Font font, const std::string& text, const Shader& shader);
	
	virtual void update() override;
	bool has_background_colour() const;
	bool has_text_border_colour() const;
	const Font& get_font() const;
	void set_font(Font font);
	const std::string& get_text() const;
	void set_text(const std::string& new_text);
	const Texture& get_texture() const;
	void set_texture(Texture texture);
	GLuint get_background_colour_uniform() const;
	GLuint get_has_background_colour_uniform() const;
	GLuint get_has_text_border_colour_uniform() const;
private:
	std::optional<Vector4F> background_colour;
	std::optional<Vector3F> text_border_colour;
	Font font;
	std::string text;
	Texture text_texture;
	GLuint background_colour_uniform, has_background_colour_uniform, text_border_colour_uniform, has_text_border_colour_uniform;
};

class Button : public TextLabel
{
public:
	Button(float x, float y, Vector4F colour, std::optional<Vector4F> background_colour, std::optional<Vector3F> text_border_colour, Font font, const std::string& text, const Shader& shader, MouseListener& mouse_listener);
	
	virtual void update() override;
	virtual bool focused() const override;
	virtual bool is_mouse_sensitive() const override;
	Command* get_on_mouse_over() const;
	Command* get_on_mouse_click() const;
	void set_on_mouse_over(Command* cmd);
	void set_on_mouse_click(Command* cmd);
	bool moused_over() const;
	bool clicked_on() const;
protected:
	MouseListener& mouse_listener;
	bool just_clicked, just_moused_over;
private:
	Command* on_mouse_over;
	Command* on_mouse_click;
};


#endif