#ifndef GUI_DISPLAY_HPP
#define GUI_DISPLAY_HPP
#include "gui.hpp"

/**
* A 2D plane rendered on the screen.
* Can contain any other gui element in its own region.
* Can have a texture bound, but does not by default.
*/
class Panel : public GUI
{
public:
	/**
	 * Construct a Panel with all specifications.
	 * @param x - X-coordinate of the Panel.
	 * @param y - Y-coordinate of the Panel.
	 * @param width - Width of the Panel.
	 * @param height - Height of the Panel.
	 * @param colour - Colour of the Panel foreground.
	 * @param shader - The shader with which to render the Panel.
	 */
	Panel(float x, float y, float width, float height, Vector4F colour, Shader& shader);

	/**
	 * Get the RGBA-encoded colour of the Panel's foreground.
	 * @return - Colour of the Panel foreground
	 */
	const Vector4F& get_colour() const;
	/**
	 * Specify the colour of the Panel's foreground.
	 * @param colour - Desired RGBA-encoded colour of the Panel's foreground
	 */
	void set_colour(Vector4F colour);
	/**
	 * Texture pointer that should be displayed on the Panel's foreground.
	 * @return - Pointer to the texture used. If no texture is being used, nullptr is returned
	 */
	const Texture* get_texture() const;
	/**
	 * Set which existing texture should be displayed on the Panel's foreground.
	 * @param texture - Pointer to the texture which should be used. If nullptr is specified, no texture will be used and the foreground colour will be used instead
	 */
	void set_texture(Texture* texture);
	/**
	 * Stop displaying a texture on this Panel and display the foreground-colour instead.
	 */
	void disable_texture();
	/**
	 * Query whether this Panel is displaying a texture on its foreground.
	 * @return - True if a texture is being displayed. Otherwise false.
	 */
	bool has_texture() const;
	/**
	 * Render the Panel, and update all children.
	 */
	virtual void update() override;
	/**
	 * Destroy the Panel, and kill all of its children.
	 */
	virtual void destroy() override;
	/**
	 * Panels cannot be focused.
	 * @return - False
	 */
	virtual bool focused() const override{return false;}
	/**
	 * Panels are not Windows.
	 * @return - False
	 */
	virtual bool is_window() const override{return false;}
	/**
	 * Panels are mouse-insensitive.
	 * @return - False
	 */
	virtual bool is_mouse_sensitive() const override{return false;}
protected:
	/**
	 * Render the Panel with the specified colour.
	 * @param colour - Colour to render the foreground of the Panel as.
	 * @param update - Whether to update all the Panel's children or not.
	 */
	void render_panel(Vector4F colour, bool update = true);

	/// Pointer to the texture which should be displayed on the Panel foreground.
	Texture* texture;
	/// Colour of the Panel foreground, if no texture is being displayed.
	Vector4F colour;
	/// Mesh representing the quad model of this Panel.
	Mesh quad;
};

/**
* Very similar to a Panel, but has additional font-rendering applied.
* Use this to write text to the screen.
*/
class TextLabel : public Panel
{
public:
	/**
	 * Construct a TextLabel with all specifications.
	 * @param x - X-coordinate of the TextLabel.
	 * @param y - Y-coordinate of the TextLabel.
	 * @param colour - Colour of the text of the TextLabel.
	 * @param background_colour - Optional background colour of the label.
	 * @param text_border_colour - Optional border colour of the text of the TextLabel.
	 * @param font - Font with which to render the text of the TextLabel.
	 * @param text - String representing the text to be displayed on the label.
	 * @param shader - Shader with which to render the TextLabel.
	 */
	TextLabel(float x, float y, Vector4F colour, std::optional<Vector4F> background_colour, std::optional<Vector3F> text_border_colour, Font font, const std::string& text, Shader& shader);

	/**
	 * Render the TextLabel, and update all children.
	 */
	virtual void update() override;
	/**
	 * Query whether this TextLabel has a background colour or not.
	 * @return - True if the TextLabel has a background colour specified. Otherwise false
	 */
	bool has_background_colour() const;
	/**
	 * Query whether this TextLabel has a text-border colour or not.
	 * @return - True if the TextLabel has a text-border colour specified. Otherwise false
	 */
	bool has_text_border_colour() const;
	/**
	 * Read the Font being used to render the text of the TextLabel.
	 * @return - Font being utilised currently
	 */
	const Font& get_font() const;
	/**
	 * Specify a new Font to be used to render the text of the TextLabel.
	 * @param font - Font to be utilised from now on
	 */
	void set_font(Font font);
	/**
	 * Read the text of the TextLabel.
	 * @return - String representing the text of the TextLabel.
	 */
	const std::string& get_text() const;
	/**
	 * Specify the new text of the TextLabel.
	 * @param new_text - String to represent the text of the TextLabel.
	 */
	void set_text(const std::string& new_text);
	/**
	 * Get the texture, representing the text of the TextLabel.
	 * @return - Texture, representing the text of the TextLabel.
	 */
	const Texture& get_texture() const;
	/**
	 * Specify a new Texture to be used, instead of the normal text texture.
	 * @param texture - New texture to be rendered on the foreground of the TextLabel.
	 */
	void set_texture(Texture texture);
private:
	using Panel::get_texture;
	using Panel::set_texture;
	using Panel::disable_texture;
	using Panel::has_texture;
	/// Optional background-colour, encoded in RGBA format.
	std::optional<Vector4F> background_colour;
	/// Optional text-border-colour, encoded in RGBA format.
	std::optional<Vector3F> text_border_colour;
	/// Font used to render the text of this TextLabel.
	Font font;
	/// String representing the text of this TextLabel.
	std::string text;
	/// Texture representing the rendered text of this TextLabel.
	Texture text_texture;
};

#endif