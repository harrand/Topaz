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

class Mesh
{
public:
	Mesh(std::string filename = "undefined.obj");
	Mesh(const Vertex* vertices, std::size_t number_of_vertices, const unsigned int* indices, std::size_t number_of_indices);
	Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
	Mesh(const Mesh& copy) = default;
	Mesh(Mesh&& move) = default;
	~Mesh();
	Mesh& operator=(const Mesh& rhs) = default;
	
	tz::graphics::model::IndexedModel get_indexed_model() const;
	const std::vector<Vector3F>& get_positions() const;
	const std::vector<Vector2F>& get_texcoords() const;
	const std::vector<Vector3F>& get_normals() const;
	const std::vector<Vector3F>& get_tangents() const;
	std::string get_file_name() const;
	void render(bool patches, GLenum mode = GL_TRIANGLES) const;
private:
	const std::string filename;
	tz::graphics::model::IndexedModel model;
	void init_mesh();
	enum class BufferTypes : std::size_t
	{
		POSITION = 0,
		TEXCOORD = 1,
		NORMAL = 2,
		INDEX = 3,
		TANGENT = 4,
		NUM_BUFFERS = 5
	};
	GLuint vertex_array_object; //vao
	std::array<GLuint, static_cast<std::size_t>(BufferTypes::NUM_BUFFERS)> vbo_buffers;
	unsigned int render_count;
};

// InstancedMesh attaches to a currently existing mesh, using its vao handle to render the mesh multiple times using the data in vectors positions, rotations and scales.
class InstancedMesh
{
public:
	InstancedMesh(Mesh* mesh, std::vector<Vector3F> positions, std::vector<Vector3F> rotations, std::vector<Vector3F> scales);
	void render(bool patches, GLenum mode = GL_TRIANGLES) const;
private:
	Mesh* mesh;
	std::vector<Vector3F> positions, rotations, scales;
	GLuint instance_positions_vbo, instance_rotations_vbo, instance_scales_vbo;
};

namespace tz
{
	namespace graphics
	{
		Mesh* find_mesh(const std::string& mesh_link, const std::vector<std::unique_ptr<Mesh>>& all_meshes);
		Mesh create_quad(float x = 0.0f, float y = 0.0f, float width = 1.0f, float height = -1.0f);
	}
}

#endif