#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <GL/glew.h>
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include "../utility.hpp"

/**
* Used to render text. Texture has a constructor taking a Font as a parameter. Use this to achieve font-rendering.
*/
class Font
{
public:
	/**
	 * Construct a font from an existing font-file.
	 * @param font_path - Path to the supported font-file.
	 * @param pixel_height - Height of the glyphs, in pixels.
	 */
	Font(const std::string& font_path, int pixel_height);
	Font(const Font& copy);
	Font(Font&& move);
	~Font();
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
	friend class Texture;
private:
	std::string font_path;
	int pixel_height;
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
	Vertex(Vector3F position, Vector2F texture_coordinate, Vector3F normal);
	Vertex(const Vertex& copy) = default;
	Vertex(Vertex&& move) = default;
	~Vertex() = default;
	Vertex& operator=(const Vertex& rhs) = default;
	
	Vector3F position;
	Vector2F texture_coordinate;
	Vector3F normal;
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
	constexpr PixelRGBA(unsigned char red = 0, unsigned char green = 0, unsigned char blue = 0, unsigned char alpha = 0): data(std::array<unsigned char, 4>({red, green, blue, alpha})){}
	Vector<4, unsigned char> data;
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
	std::vector<Pixel> pixels;
	int width, height;
};

namespace tz
{
	namespace graphics
	{
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
		inline void initialise();
		/**
		* Terminates and destroys all graphics components of Topaz. tz::terminate will invoke this automatically.
		* Only use this function if you do not wish to use tz::terminate to terminate all features, but instead cherry-pick components like graphics.
		*/
		inline void terminate();
		/**
		* Currently used only for Wavefront OBJ Models. To load an OBJ model in Topaz, invoke OBJModel::toIndexedModel to receive an instance of IndexedModel.
		* A Topaz Mesh constructor can take an IndexedModel as a parameter.
		*/
		namespace model
		{
		    /**
		     * Representation of indices for a vertex, texture-coordinate and normal.
		     */
			class OBJIndex
			{
			public:
				unsigned int vertex_index;
				unsigned int uv_index;
				unsigned int normal_index;
				bool operator<(const OBJIndex& rhs) const { return vertex_index < rhs.vertex_index; }
			};
			/**
			 * Barebones representation of a 3D model.
			 */
			class IndexedModel
			{
			public:
				std::vector<Vector3F> positions;
				std::vector<Vector2F> texcoords;
				std::vector<Vector3F> normals;
				std::vector<Vector3F> tangents;
				std::vector<unsigned int> indices;
				void calculate_normals();
				void calculate_tangents();
			};
			
			class OBJModel
			{
			public:
				std::vector<OBJIndex> obj_indices;
				std::vector<Vector3F> vertices;
				std::vector<Vector2F> uvs;
				std::vector<Vector3F> normals;
				bool has_uvs;
				bool has_normals;

				/**
				 * Load an existing OBJ file.
				 * @param file_name - Path to the existing OBJ-formatted 3D model.
				 * The model must have triangular faces. OBJ Materials (mtl files) are not supported.
				 */
				OBJModel(const std::string& file_name);
				
				IndexedModel to_indexed_model();
			private:
				unsigned int find_last_vertex_index(const std::vector<OBJIndex*>& index_lookup, const OBJIndex* current_index, const IndexedModel& result);
				void create_obj_face(const std::string& line);
				
				Vector2F parse_obj_vector_2f(const std::string& line);
				Vector3F parse_obj_vector_3f(const std::string& line);
				OBJIndex parse_obj_index(const std::string& token, bool* has_uvs, bool* has_normals);
			};
		}
	}
}

#include "graphics.inl"
#endif