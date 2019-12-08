#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include "GL/glew.h"
#include "data/vector.hpp"
#include <mutex>

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

namespace tz
{
	namespace graphics
	{
		enum class BufferBit : GLbitfield
		{
			COLOUR = GL_COLOR_BUFFER_BIT,
			DEPTH = GL_DEPTH_BUFFER_BIT,
			COLOUR_AND_DEPTH = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
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
		inline void initialise();
		/**
		* Terminates and destroys all graphics components of Topaz. tz::terminate will invoke this automatically.
		* Only use this function if you do not wish to use tz::terminate to terminate all features, but instead cherry-pick components like graphics.
		*/
		inline void terminate();
		inline void scene_render_mode();
		inline void gui_render_mode();
		inline void enable_wireframe_render(bool wireframes, float wireframe_width = 1.0f);
		inline bool ready();
	}
}

#include "graphics.inl"
#endif