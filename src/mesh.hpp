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
	
	tz::graphics::model::IndexedModel getIndexedModel() const;
	const std::vector<Vector3F>& getPositions() const;
	const std::vector<Vector2F>& getTexcoords() const;
	const std::vector<Vector3F>& getNormals() const;
	std::string getFileName() const;
	void render(bool patches, GLenum mode = GL_TRIANGLES) const;
private:
	const std::string filename;
	tz::graphics::model::IndexedModel model;
	void initMesh();
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

namespace tz
{
	namespace graphics
	{
		Mesh* findMesh(const std::string& mesh_link, const std::vector<std::unique_ptr<Mesh>>& all_meshes);
		Mesh createQuad(float x = 0.0f, float y = 0.0f, float width = 1.0f, float height = -1.0f);
	}
}

#endif