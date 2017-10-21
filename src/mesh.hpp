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
		TANGENT = 3,
		INDEX = 4,
		NUM_BUFFERS
	};
	GLuint vertex_array_object; //vao
	std::array<GLuint, static_cast<std::size_t>(BufferTypes::NUM_BUFFERS)> vbo_buffers;
	unsigned int render_count;
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