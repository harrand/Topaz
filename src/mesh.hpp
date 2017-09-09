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
	Mesh(const Vertex* vertices, unsigned int number_of_vertices, const unsigned int* indices, unsigned int number_of_indices);
	Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
	Mesh(const Mesh& copy) = default;
	Mesh(Mesh&& move) = default;
	~Mesh();
	Mesh& operator=(const Mesh& rhs) = default;
	
	IndexedModel getIndexedModel() const;
	
	const std::vector<Vector3F>& getPositions() const;
	const std::vector<Vector2F>& getTexcoords() const;
	const std::vector<Vector3F>& getNormals() const;
	
	std::string getFileName() const;
	
	void render(bool patches) const;
	
	static Mesh* getFromLink(const std::string& mesh_link, const std::vector<std::unique_ptr<Mesh>>& all_meshes);
private:
	const std::string filename;
	IndexedModel model;
	void initMesh();
	enum class BufferTypes : unsigned int
	{
		POSITION = 0,
		TEXCOORD = 1,
		NORMAL = 2,
		INDEX = 3,
		TANGENT = 4,
		NUM_BUFFERS = 5
	};
	GLuint vertex_array_object; //vao
	GLuint vbo_buffers[static_cast<unsigned int>(BufferTypes::NUM_BUFFERS)]; //vbo array
	unsigned int render_count;
};

namespace tz
{
	namespace graphics
	{
		Mesh createQuad();
	}
}

#endif