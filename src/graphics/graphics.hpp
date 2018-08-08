#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <GL/glew.h>
#include "data/vector.hpp"
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"

/**
* Used to render text. Texture has a constructor taking a Font as a parameter. Use this to achieve font-rendering.
*/
class Font
{
public:
	enum class Style : int
	{
		NORMAL = TTF_STYLE_NORMAL,
		BOLD = TTF_STYLE_BOLD,
		ITALIC = TTF_STYLE_ITALIC,
		UNDERLINE = TTF_STYLE_UNDERLINE,
		STRIKED = TTF_STYLE_STRIKETHROUGH,
	};
	/**
	 * Construct a font from an existing font-file.
	 * @param font_path - Path to the supported font-file.
	 * @param pixel_height - Height of the glyphs, in pixels.
	 */
	Font(const std::string& font_path, int pixel_height);
	/**
	 * Construct a Font from another Font's font-file.
	 * @param copy - The Font whose font-file should be used.
	 */
	Font(const Font& copy);
	/**
	 * Construct a Font, taking control of an existing Font's font-data.
	 * @param move - The Font whose font-data should be taken.
	 */
	Font(Font&& move);
	/**
	 * Safely dispose of font-data.
	 */
	~Font();
	/**
	 * Copy-and-swap idiom copy-assignment.
	 * @param rhs - Font whose data should be copied
	 * @return - The resultant font
	 */
	Font& operator=(Font rhs);
	/**
	 * Trivial move-assignment.
	 * @param rhs - Font whose data members should be taken.
	 * @return - The resultant font.
	 */
	Font& operator=(Font&& rhs);
	/**
	 * Get the height of glyphs, in pixels.
	 * @return - Height of glyphs, in pixels.
	 */
	int get_pixel_height() const;
	/**
	 * Retrieve the filename of the file used to load this Font.
	 * @return - Filename of the source font.
	 */
	const std::string& get_path() const;
	Font::Style get_style() const;
	void set_style(Font::Style style);
	bool has_outline() const;
	int get_outline_size() const;
	void set_outline_size(int outline);
	friend class Texture;
private:
	/// Path to the font-file containing font-data.
	std::string font_path;
	/// Height of glyphs, in pixels.
	int pixel_height;
	/// SDL_ttf graphical font-data.
	TTF_Font* font_handle;
};

/**
* Holds vertex data as POD.
*/
class Vertex
{
public:
	/**
	 * Construct a Vertex directly from a 3-dimensional position, a 2-dimensional texture-coordinate (UV) and a 3-dimensional normal Vector.
	 * @param position
	 * @param texture_coordinate
	 * @param normal
	 */
	Vertex(Vector3F position, Vector2F texture_coordinate, Vector3F normal, Vector3F tangent = {});
    bool operator==(const Vertex& rhs) const;

	/// Position of the Vertex, in model-space.
	Vector3F position;
	/// Texture coordinate of the Vertex (UV).
	Vector2F texture_coordinate;
	/// Normal Vector of the Vertex.
	Vector3F normal;
	/// Tangent Vector of the Vertex.
	Vector3F tangent;
};

/**
* Representation of Pixel Data in RGBA format.
*/
class PixelRGBA
{
public:
	/**
	 * Construct an RGBA-coded pixel directly from colour data.
	 * @param red - Intensity of the red colour, from 0-255.
	 * @param green - Intensity of the green colour, from 0-255.
	 * @param blue - Intensity of the blue colour, from 0-255.
	 * @param alpha - Intensity of the alpha component (typically transparency), from 0-255.
	 */
	constexpr PixelRGBA(unsigned char red = 0, unsigned char green = 0, unsigned char blue = 0, unsigned char alpha = 255): data(std::array<unsigned char, 4>({red, green, blue, alpha})){}
	/// 4-dimensional Vector containing the RGBA colour intensities.
	Vector<4, unsigned char> data;
};

class PixelRGBAHDR
{
public:
	constexpr PixelRGBAHDR(float red = 0.0f, float green = 0.0f, float blue = 0.0f, float alpha = 255.0f): data(std::array<float, 4>({red, green, blue, alpha})){}
	Vector<4, float> data;
};

class PixelDepth
{
public:
	/**
	 * Construst a Pixel storing a depth value. Used in depth-textures.
	 * @param depth - Depth of the pixel, from 0.0-1.0
	 */
	constexpr PixelDepth(float depth): data(std::array<float, 1>({depth})){}
	Vector<1, float> data;
};

/**
* Bitmap representing Pixel data in any format.
* Topaz uses PixelRGBA as the template parameter, but you may provide any valid class with a public Vector4<T> called 'data'.
*/
template<class Pixel = PixelRGBA>
class Bitmap
{
public:
	/**
	 * Construct a Bitmap directly from a vector of Pixels.
	 * @param pixels - Pixel data to construct the bitmap.
	 * @param width - Width of the bitmap, in pixels.
	 * @param height - Height of the bitmap, in pixels.
	 */
	Bitmap(std::vector<Pixel> pixels = std::vector<Pixel>(), int width = 0, int height = 0): pixels(pixels), width(width), height(height){}
	/// Container for all pixels.
	std::vector<Pixel> pixels;
	/// Width of the Bitmap, in pixels.
	int width;
	/// Height of the Bitmap, in pixels.
	int height;
};

namespace tz
{
	namespace graphics
	{
		enum class BufferBit : GLbitfield
		{
			COLOUR = GL_COLOR_BUFFER_BIT,
			DEPTH = GL_DEPTH_BUFFER_BIT,
			STENCIL = GL_STENCIL_BUFFER_BIT,
			COLOUR_AND_DEPTH = (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT),
		};
		/**
		* Global Variables. These are handled when the first Window is initialised. Editing these is likely to lead to horrible crashing. Leave these bools alone.
		*/
		extern bool has_context;
		extern bool initialised;
		/**
		* Initialises the graphics component of Topaz. An OpenGL window-context MUST exist before this is executed.
		* Topaz Window constructor invokes this upon first invocation.
		* If you are not using Topaz windows, you will have to invoke this yourself.
		* If you are using Topaz windows, DO NOT TOUCH THIS.
		*/
		inline void initialise(bool print_progress = false);
		/**
		* Terminates and destroys all graphics components of Topaz. tz::terminate will invoke this automatically.
		* Only use this function if you do not wish to use tz::terminate to terminate all features, but instead cherry-pick components like graphics.
		*/
		inline void terminate(bool print_progress = false);
		inline void scene_render_mode();
		inline void gui_render_mode();
		inline void enable_wireframe_render(bool wireframes);
	}
}

#include "graphics.inl"
#endif