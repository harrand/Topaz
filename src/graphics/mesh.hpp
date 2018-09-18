#ifndef MESH_HPP
#define MESH_HPP
#include "graphics/graphics.hpp"
#include "data/matrix.hpp"
#include <vector>
#include <string>
#include <memory>
#include <array>
#include <set>

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

/**
* Lowest-level renderable class that Topaz offers.
* All renderable Topaz classes such as Object3D contain these.
* Holds 3D vertex data, from a Wavefront OBJ model, for example.
* Use this if you have an existing shader you can use to draw manually.
*/
class Mesh
{
public:
	/**
	 * Construct a Mesh from an existing 3D model file (.obj).
	 * @param filename - Path to the external 3D model file
	 * @param scene_index - The index of the mesh in the scene to load.
	 * Note: To load all meshes from a file, use tz::graphics::load_all_meshes(filename) instead.
	 */
	Mesh(std::string filename, std::size_t scene_index = 0);
	/**
	 * Construct a Mesh from C-style arrays.
	 * @param vertices - Pointer to the first element of the Vertex array
	 * @param number_of_vertices  - Size of the Vertex array, in elements
	 * @param indices - Pointer to the first element of the index array
	 * @param number_of_indices - Size of the index array, in elements
	 */
	Mesh(const Vertex* vertices, std::size_t number_of_vertices, const unsigned int* indices, std::size_t number_of_indices);
	/**
	 * Construct a Mesh from containers of vertices and indices respectively.
	 * @param vertices - The container of vertices
	 * @param indices - The container of indices
	 */
	Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
	/// Performs a deep-VRAM-copy. If shared ownership is needed, consider using an AssetBuffer.
	Mesh(const Mesh& copy);
	Mesh(Mesh&& move);
	/**
	 * Safely dispose of loaded Mesh data.
	 */
	virtual ~Mesh();
	Mesh& operator=(Mesh rhs);
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
	const std::vector<unsigned int>& get_indices() const;
	/**
	 * Render the Mesh, using the currently-bound Shader.
	 * @param patches - Whether to use Patches as the OpenGL primitive or not
	 * @param mode - Which primitive to use (if patches is true, uses patches with the same number of vertices as this polygon)
	 */
	virtual void render(bool patches, GLenum mode = GL_TRIANGLES) const;
	/**
	 * Equate this Mesh with another.
	 * @param rhs - The other Mesh to compare to
	 * @return - True if the Meshes are equal. False otherwise
	 */
	bool operator==(const Mesh& rhs) const;
protected:
    static void swap(Mesh& lhs, Mesh& rhs);
    /// List of all vertex-data.
	std::vector<Vertex> vertices;
	/// OpenGL VAO handle.
	GLuint vertex_array_object;
	/// Array of OpenGL Vertex Buffer Object buffers
	std::array<GLuint, static_cast<std::size_t>(tz::graphics::BufferTypes::NUM_BUFFERS)> vbo_buffers;
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
class InstancedMesh : public Mesh
{
public:
	/**
	 * Construct an InstancedMesh from an external 3D model file (.obj format).
	 * @param filename - Path to the external 3D model file. This mesh will store the first mesh in the file.
	 * @param positions - Container of 3-dimensional Vectors representing instance position offsets, in world-space
	 * @param rotations - Container of 3-dimensional Vectors representing instance rotation offsets, in euler-angles
	 * @param scales - Container of 3-dimensional Vectors representing instance rotation offsets, in the three spatial dimensions XYZ
	 */
	InstancedMesh(std::string filename, std::vector<Vector3F> positions, std::vector<Vector3F> rotations, std::vector<Vector3F> scales, bool dynamic_transform = false);
	InstancedMesh(const Mesh& uninstanced_copy, std::vector<Vector3F> positions, std::vector<Vector3F> rotations, std::vector<Vector3F> scales, bool dynamic_transform = false);
	InstancedMesh(const InstancedMesh& copy);
	InstancedMesh(InstancedMesh&& move);
	/**
	 * Safely dispose of loaded InstancedMesh data.
	 */
	virtual ~InstancedMesh();
	InstancedMesh& operator=(InstancedMesh rhs);

	/**
	 * Get a container of all the 3-dimensional Vectors representing instance position offsets, in world-space.
	 * @return - Container of instance position offsets
	 */
	const std::vector<Vector3F>& get_instance_positions() const;
	/**
	 * Get a container of all the 3-dimensional Vectors representing instance rotation offsets, in euler-angles.
	 * @return - Container of instance rotation offsets
	 */
	const std::vector<Vector3F>& get_instance_rotations() const;
	/**
	 * Get a container of all the 3-dimensional Vectors representing instance scale offsets, in the three spatial dimensions XYZ.
	 * @return - Container of instance scale offsets
	 */
	const std::vector<Vector3F>& get_instance_scales() const;
	bool set_instance_position(std::size_t instance_id, Vector3F position);
	bool set_instance_rotation(std::size_t instance_id, Vector3F rotation);
	bool set_instance_scale(std::size_t instance_id, Vector3F scale);
    /**
     * Get a container of all the 4-dimensional square-matrices representing each corresponding model-matrix.
     * @return - Container of all model-matrices
     */
    const std::vector<Matrix4x4>& get_model_matrices() const;
	/**
	 * Get the number of additional instances (excluding the normal mesh).
	 * @return - Number of instances
	 */
	std::size_t get_instance_quantity() const;
	/**
	 * Render the Mesh, using the currently-bound Shader.
	 * @param patches - Whether to use Patches as the OpenGL primitive or not
	 * @param mode - Which primitive to use (if patches is true, uses patches with the same number of vertices as this polygon)
	 */
	virtual void render(bool patches, GLenum mode = GL_TRIANGLES) const override;
private:
	void update_instance(std::size_t instance_id);
    static void swap(InstancedMesh& lhs, InstancedMesh& rhs);
	/// Instance offsets.
	std::vector<Vector3F> positions, rotations, scales;
    /// Model Matrices.
    std::vector<Matrix4x4> models;
	/// Number of instances.
	std::size_t instance_quantity;
    bool dynamic_transform;
	/// Underlying OpenGL VBO handles.
	GLuint model_matrix_x_vbo, model_matrix_y_vbo, model_matrix_z_vbo, model_matrix_w_vbo;
};

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