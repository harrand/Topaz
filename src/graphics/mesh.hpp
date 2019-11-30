#ifndef MESH_HPP
#define MESH_HPP
#include "graphics/graphics.hpp"
#include "data/matrix.hpp"
#include <vector>
#include <string>
#include <memory>
#include <array>
#include <set>
#include "assimp/mesh.h"
#include "gl/vertex_buffer.hpp"

namespace tz::graphics
{
	enum class BufferTypes : std::size_t
	{
		POSITION = 0,
		TEXCOORD = 1,
		NORMAL = 2,
		TANGENT = 3,
		INDEX = 4,
		NUM_BUFFERS = 5
	};
}

#ifdef TOPAZ_OPENGL
class Model;
namespace tz::gl
{
	enum class StandardAttribute : GLuint
	{
		POSITION = 0,
		TEXCOORD = 1,
		NORMAL = 2,
		TANGENT = 3,
		INSTANCE_MODEL_X_ROW = 4,
		INSTANCE_MODEL_Y_ROW = 5,
		INSTANCE_MODEL_Z_ROW = 6,
		INSTANCE_MODEL_W_ROW = 7
	};

	class OGLMesh
	{
	public:
		/**
		 * Construct a Mesh from an existing 3D model file (.obj).
		 * @param filename - Path to the external 3D model file
		 * @param scene_index - The index of the mesh in the scene to load.
		 * Note: To load all meshes from a file, use tz::graphics::load_all_meshes(filename) instead.
		 */
		OGLMesh(std::string filename, std::size_t scene_index = 0);
		/**
		 * Construct a Mesh from C-style arrays.
		 * @param vertices - Pointer to the first element of the Vertex array
		 * @param number_of_vertices  - Size of the Vertex array, in elements
		 * @param indices - Pointer to the first element of the index array
		 * @param number_of_indices - Size of the index array, in elements
		 */
		OGLMesh(const Vertex *vertices, std::size_t number_of_vertices, const unsigned int *indices,
				std::size_t number_of_indices);
		/**
		 * Construct a Mesh from containers of vertices and indices respectively.
		 * @param vertices - The container of vertices
		 * @param indices - The container of indices
		 */
		OGLMesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices);
		/// Performs a deep-VRAM-copy. If shared ownership is needed, consider using an AssetBuffer.
		OGLMesh(const OGLMesh &copy);
		OGLMesh(OGLMesh &&move);
		/**
		 * Safely dispose of loaded Mesh data.
		 */
		virtual ~OGLMesh() = default;
		OGLMesh &operator=(OGLMesh rhs);
		/**
		 * Get a container of 3-dimensional Vectors representing the position of each Vertex, in model-space.
		 * @return - Container of Vertex positions
		 */
		std::vector<Vector3F> get_positions() const;
		/**
		 * Get a container of 2-dimensional Vectors representing the texture-coordinate of each Vertex (UV).
		 * @return - Container of Vertex texture-coordinates
		 */
		std::vector<Vector2F> get_texcoords() const;
		/**
		 * Get a container of 3-dimensional Vectors representing the normal Vector of each Vertex, in model-space.
		 * @return - Container of Vertex normal Vectors
		 */
		std::vector<Vector3F> get_normals() const;
		/**
		 * Get a container of 3-dimensional Vectors representing the tangent Vector of each Vertex, in model-space.
		 * @return - Container of Vertex tangent Vectors
		 */
		std::vector<Vector3F> get_tangents() const;
		/**
		 * Get a container of unsigned integers representing the indices of all vertices.
		 * @return - Indices of the Vertices
		 */
		const std::vector<unsigned int> &get_indices() const;
		/**
		 * Render the Mesh, using the currently-bound Shader.
		 * @param patches - Whether to use Patches as the OpenGL primitive or not
		 * @param mode - Which primitive to use (if patches is true, uses patches with the same number of vertices as this polygon)
		 */
		virtual void render(bool patches) const;
		/**
		 * Equate this Mesh with another.
		 * @param rhs - The other Mesh to compare to
		 * @return - True if the Meshes are equal. False otherwise
		 */
		bool operator==(const OGLMesh &rhs) const;
		friend class ::Model;
	protected:
		OGLMesh();
		OGLMesh(const aiMesh *assimp_mesh);
		static void swap(OGLMesh &lhs, OGLMesh &rhs);
		/// List of all vertex-data.
		std::vector<Vertex> vertices;
		/// OpenGL-specific VAO.
		tz::gl::OGLVertexArray vertex_array;
		/// Element Array Buffer, essentially.
		std::vector<unsigned int> indices;
	private:
		/// Initialise the underlying mesh data container.
		void init_mesh();
	};

/**
* Like a normal mesh, but supports OpenGL instancing.
* Use this if you want to render the same mesh very many times at once with little attribute changes.
* This class is abstracted away by tz::graphics::batch in object.hpp.
*/
	class OGLInstancedMesh : public OGLMesh
	{
	public:
		/**
		 * Construct an OGLInstancedMesh from an external 3D model file (.obj format).
		 * @param filename - Path to the external 3D model file. This mesh will store the first mesh in the file.
		 * @param positions - Container of 3-dimensional Vectors representing instance position offsets, in world-space
		 * @param rotations - Container of 3-dimensional Vectors representing instance rotation offsets, in euler-angles
		 * @param scales - Container of 3-dimensional Vectors representing instance rotation offsets, in the three spatial dimensions XYZ
		 */
		OGLInstancedMesh(std::string filename, std::vector<Vector3F> positions, std::vector<Vector3F> rotations, std::vector<Vector3F> scales, bool dynamic_transform = false);
		OGLInstancedMesh(const OGLMesh &uninstanced_copy, std::vector<Vector3F> positions, std::vector<Vector3F> rotations, std::vector<Vector3F> scales, bool dynamic_transform = false);
		OGLInstancedMesh(const OGLInstancedMesh &copy);
		OGLInstancedMesh(OGLInstancedMesh &&move);
		/**
		 * Safely dispose of loaded OGLInstancedMesh data.
		 */
		virtual ~OGLInstancedMesh() = default;
		OGLInstancedMesh &operator=(OGLInstancedMesh rhs);
		/**
		 * Get a container of all the 3-dimensional Vectors representing instance position offsets, in world-space.
		 * @return - Container of instance position offsets
		 */
		const std::vector<Vector3F> &get_instance_positions() const;
		/**
		 * Get a container of all the 3-dimensional Vectors representing instance rotation offsets, in euler-angles.
		 * @return - Container of instance rotation offsets
		 */
		const std::vector<Vector3F> &get_instance_rotations() const;
		/**
		 * Get a container of all the 3-dimensional Vectors representing instance scale offsets, in the three spatial dimensions XYZ.
		 * @return - Container of instance scale offsets
		 */
		const std::vector<Vector3F> &get_instance_scales() const;
		bool set_instance_position(std::size_t instance_id, Vector3F position);
		bool set_instance_rotation(std::size_t instance_id, Vector3F rotation);
		bool set_instance_scale(std::size_t instance_id, Vector3F scale);
		/**
		 * Get a container of all the 4-dimensional square-matrices representing each corresponding model-matrix.
		 * @return - Container of all model-matrices
		 */
		const std::vector<Matrix4x4> &get_model_matrices() const;
		/**
		 * Get the number of additional instances (excluding the normal mesh).
		 * @return - Number of instances
		 */
		std::size_t get_instance_quantity() const;
		/**
		 * Render the OGLMesh, using the currently-bound Shader.
		 * @param patches - Whether to use Patches as the OpenGL primitive or not
		 * @param mode - Which primitive to use (if patches is true, uses patches with the same number of vertices as this polygon)
		 */
		virtual void render(bool patches) const override;
	private:
		void update_instance(std::size_t instance_id);
		static void swap(OGLInstancedMesh &lhs, OGLInstancedMesh &rhs);
		/// Instance offsets.
		std::vector<Vector3F> positions, rotations, scales;
		/// Model Matrices.
		std::vector<Matrix4x4> models;
		/// Number of instances.
		std::size_t instance_quantity;
		/// Used for optimisation. Stores whether we expect instances to have their values changed often.
		bool dynamic_transform;
		/// Underlying OpenGL VBO handles.
		tz::gl::OGLVertexBuffer *model_matrix_x_vbo, *model_matrix_y_vbo, *model_matrix_z_vbo, *model_matrix_w_vbo;
	};
}

using Mesh = tz::gl::OGLMesh;
using InstancedMesh = tz::gl::OGLInstancedMesh;
#endif

namespace tz
{
	namespace graphics
	{
		std::vector<Mesh> load_all_meshes(const std::string& filename);
		/**
		 * Query whether the specified Mesh is an InstancedMesh.
		 * @param mesh - The Mesh to query
		 * @return - True if the Mesh is an InstancedMesh. Otherwise false
		 */
		bool is_instanced(const Mesh* mesh);
		/**
		 * Create a single Quad, like so:
		 * *--------* [x + width, y + height]
		 * |		|
		 * |	*	|
		 * | [x, y] |
		 * *--------*
		 * [x - width, y - height]
		 *
		 * @param x - X-position of the Quad centre, in model-space
		 * @param y - Y-position of the Quad centre, in model-space
		 * @param width - Distance between Quad centre to left/right of Quad, in model-space
		 * @param height - Distance between Quad centre to top/bottom of Quad, in model-space
		 * @return - The constructed Quad mesh
		 */
		Mesh create_quad(float x = 0.0f, float y = 0.0f, float width = 1.0f, float height = -1.0f);
		/**
		 * Create a simple cube mesh.
		 * @return - A cube mesh
		 */
		Mesh create_cube();
	}
}

#endif