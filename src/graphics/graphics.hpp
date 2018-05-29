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
	Vertex(Vector3F position, Vector2F texture_coordinate, Vector3F normal);

	/// Position of the Vertex, in model-space.
	Vector3F position;
	/// Texture coordinate of the Vertex (UV).
	Vector2F texture_coordinate;
	/// Normal Vector of the Vertex.
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
	/// 4-dimensional Vector containing the RGBA colour intensities.
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
		inline void scene_render_mode();
		inline void gui_render_mode();
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
				/// Index of the Vertex position.
				unsigned int vertex_index;
				/// Index of the Vertex texture-coordinate.
				unsigned int uv_index;
				/// Index of the Vertex normal Vector.
				unsigned int normal_index;
				/**
				 * Compare this OBJIndex to another, via index of the Vertex position.
				 * @param rhs - The OBJIndex to be compared with.
				 * @return - True if the index of this Vertex position is greater than the parameter's index.
				 */
				bool operator<(const OBJIndex& rhs) const { return vertex_index < rhs.vertex_index; }
			};
			/**
			 * Barebones representation of a 3D model.
			 */
			class IndexedModel
			{
			public:
				/// Container for all Vertex positions. Each position is in model-space.
				std::vector<Vector3F> positions;
				/// Container for all Vertex texture-coordinates (UVs).
				std::vector<Vector2F> texcoords;
				/// Container for all Vertex normal Vectors.
				std::vector<Vector3F> normals;
				/// Container for all Vertex tangent Vectors.
				std::vector<Vector3F> tangents;
				/// Container for all Vertex indices.
				std::vector<unsigned int> indices;
				/**
				 * Calculate and populate the container for all Vertex normal Vectors.
				 */
				void calculate_normals();
				/**
				 * Calculate the populate the container for all Vertex tangent Vectors.
				 */
				void calculate_tangents();
			};

            /**
             * Imported OBJ wavefront 3D model structure.
             */
			class OBJModel
			{
			public:
				/// Container for all the Vertices' indices of the model.
				std::vector<OBJIndex> obj_indices;
				/// Container for all the Vertex positions, each in model-space.
				std::vector<Vector3F> vertices;
				/// Container for all the Vertex texture-coordinates.
				std::vector<Vector2F> uvs;
				/// Container for all the Vertex normal Vectors.
				std::vector<Vector3F> normals;
				/// Stores whether this OBJModel is storing texture-coordinates.
				bool has_uvs;
				/// Stores whether this OBJModel is storing normal Vectors.
				bool has_normals;

				/**
				 * Load an existing OBJ file.
				 * @param file_name - Path to the existing OBJ-formatted 3D model.
				 * The model must have triangular faces. OBJ Materials (mtl files) are not supported.
				 */
				OBJModel(const std::string& file_name);

				/// Collaborate data and create the barebones IndexedModel.
				IndexedModel to_indexed_model();
			private:
				/// Helper function.
				unsigned int find_last_vertex_index(const std::vector<OBJIndex*>& index_lookup, const OBJIndex* current_index, const IndexedModel& result);
				/// Helper function.
				void create_obj_face(const std::string& line);

				/// Helper function.
				Vector2F parse_obj_vector_2f(const std::string& line);
				/// Helper function.
				Vector3F parse_obj_vector_3f(const std::string& line);
				/// Helper function.
				OBJIndex parse_obj_index(const std::string& token, bool* has_uvs, bool* has_normals);
			};
		}
	}
}

#include "graphics.inl"
#endif