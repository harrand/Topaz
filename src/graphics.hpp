#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include "GL/glew.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include "utility.hpp"

/*
	Used to render text. Texture has a constructor taking a Font as a parameter. Use this to achieve font-rendering.
*/
class Font
{
public:
	Font(const std::string& font_path, int pixel_height);
	Font(const Font& copy);
	Font(Font&& move);
	~Font();
	Font& operator=(Font&& rhs); // move assignment operator
	
	int get_pixel_height() const;
	std::string_view get_path() const;
	friend class Texture;
private:
	std::string font_path;
	int pixel_height;
	TTF_Font* font_handle;
};

/*
	Holds vertex data. Essentially a POD if it weren't for the getters.
*/
class Vertex
{
public:
	Vertex(Vector3F position, Vector2F texture_coordinate, Vector3F normal);
	Vertex(const Vertex& copy) = default;
	Vertex(Vertex&& move) = default;
	~Vertex() = default;
	Vertex& operator=(const Vertex& rhs) = default;
	
	Vector3F position;
	Vector2F texture_coordinate;
	Vector3F normal;
};

/*
	Representation of Pixel Data in RGBA format.
*/
class PixelRGBA
{
public:
	PixelRGBA(unsigned char red = 0, unsigned char green = 0, unsigned char blue = 0, unsigned char alpha = 0);
	Vector4<unsigned char> data;
};

/*
	Bitmap representing Pixel data in any format. Topaz uses PixelRGBA as the template parameter, but you may provide any valid class with a public Vector4<T> called 'data'.
*/
template<class Pixel>
class Bitmap
{
public:
	Bitmap(std::vector<Pixel> pixels = std::vector<Pixel>(), int width = 0, int height = 0): pixels(pixels), width(width), height(height){}
	std::vector<Pixel> pixels;
	int width, height;
};

namespace tz
{
	namespace graphics
	{
		// Global Variables. These are handled when the first Window is initialised. Editing these is likely to lead to horrible crashing. Leave these bools alone.
		extern bool has_context;
		extern bool initialised;
		// Window constructor invokes this upon first invocation. If you are not using Topaz windows, you will have to invoke this yourself. If you are, do not touch this.
		inline void initialise();
		// Much safer to use. tz::terminate will invoke this automatically. Only use this function if you do not wish to use tz::terminate to terminate all features.
		inline void terminate();
		/*
			Currently used only for Wavefront OBJ Models. To load an OBJ model in Topaz, invoke OBJModel::toIndexedModel to receive an instance of IndexedModel.
		*/
		namespace model
		{
			class OBJIndex
			{
			public:
				unsigned int vertex_index;
				unsigned int uv_index;
				unsigned int normal_index;
				bool operator<(const OBJIndex& rhs) const { return vertex_index < rhs.vertex_index; }
			};
			
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