#ifndef MESH_HPP
#define MESH_HPP
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <vector>
#include <string>
#include <memory>
#include <array>
#include "graphics.hpp"
#include "matrix.hpp"

/*
	Lowest-level renderable class that Topaz offers. All renderable Topaz classes such as Object contain these. Holds 3D vertex data, from a Wavefront OBJ model, for example. Use this if you want to render a mesh you made in Blender or something.
*/
class Mesh
{
public:
	Mesh(std::string filename = "undefined.obj");
	Mesh(const Vertex* vertices, std::size_t number_of_vertices, const unsigned int* indices, std::size_t number_of_indices);
	Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
	Mesh(const Mesh& copy) = default;
	Mesh(Mesh&& move) = default;
	virtual ~Mesh();
	Mesh& operator=(const Mesh& rhs) = default;
	
	tz::graphics::model::IndexedModel get_indexed_model() const;
	const std::vector<Vector3F>& get_positions() const;
	const std::vector<Vector2F>& get_texcoords() const;
	const std::vector<Vector3F>& get_normals() const;
	const std::vector<Vector3F>& get_tangents() const;
	std::string get_file_name() const;
	virtual void render(bool patches, GLenum mode = GL_TRIANGLES) const;
protected:
	const std::string filename;
	tz::graphics::model::IndexedModel model;
	void init_mesh();
	enum class BufferTypes : std::size_t
	{
		POSITION = 0,
		TEXCOORD = 1,
		NORMAL = 2,
		TANGENT = 3,
		INDEX = 4,
		NUM_BUFFERS = 5
	};
	GLuint vertex_array_object; //vao
	std::array<GLuint, static_cast<std::size_t>(BufferTypes::NUM_BUFFERS)> vbo_buffers;
	unsigned int render_count;
};

/*
	Like a normal mesh, but supports OpenGL instancing. Use this if you want to render the same mesh very many times at once with little attribute changes.
*/
class InstancedMesh : public Mesh
{
public:
	InstancedMesh(std::string filename, std::vector<Vector3F> positions, std::vector<Vector3F> rotations, std::vector<Vector3F> scales);
	InstancedMesh(const InstancedMesh& copy) = default;
	InstancedMesh(InstancedMesh&& move) = default;
	~InstancedMesh();
	InstancedMesh& operator=(const InstancedMesh& rhs) = default;
	
	const std::vector<Vector3F>& get_instance_positions() const;
	const std::vector<Vector3F>& get_instance_rotations() const;
	const std::vector<Vector3F>& get_instance_scales() const;
	std::size_t get_instance_quantity() const;
	virtual void render(bool patches, GLenum mode = GL_TRIANGLES) const override;
private:
	std::vector<Vector3F> positions, rotations, scales;
	std::size_t instance_quantity;
	GLuint positions_instance_vbo, rotations_instance_vbo, scales_instance_vbo;
};

namespace tz
{
	namespace graphics
	{
		bool is_instanced(const Mesh* mesh);
		Mesh* find_mesh(const std::string& mesh_link, const std::vector<std::unique_ptr<Mesh>>& all_meshes);
		Mesh create_quad(float x = 0.0f, float y = 0.0f, float width = 1.0f, float height = -1.0f);
	}
}

#endif