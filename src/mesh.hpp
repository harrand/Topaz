#ifndef MESH_HPP
#define MESH_HPP
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <vector>
#include <string>
#include <memory>
#include "glew.h"
#include "model_loader.hpp"

class Vertex
{
public:
	Vertex(Vector3F position, Vector2F texcoord, Vector3F normal);
	Vertex(const Vertex& copy) = default;
	Vertex(Vertex&& move) = default;
	Vertex& operator=(const Vertex& rhs) = default;
	
	Vector3F position;
	Vector2F texcoord;
	Vector3F normal;
};

class Mesh
{
public:
	Mesh(Vertex* vertices, unsigned int numVertices, unsigned int* indices, unsigned int numIndices);
	Mesh(const Mesh& copy) = delete;
	Mesh(Mesh&& move) = delete;
	Mesh& operator=(const Mesh& rhs) = delete;
	// Don't want any of these because I only ever want one instance of mesh per model. Allowing us to copy and move instances around will be inefficient and pointless.
	
	Mesh(std::string filename = "./res/models/undefined.obj");
	~Mesh();
	IndexedModel getIndexedModel() const;
	std::string getFileName() const;
	void render() const;
	static Mesh* getFromLink(const std::string& meshLink, const std::vector<std::unique_ptr<Mesh>>& allMeshes);
private:
	IndexedModel model;
	const std::string filename;
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
	GLuint vertexArrayObject; //vao
	GLuint vertexArrayBuffers[(unsigned int)BufferTypes::NUM_BUFFERS]; //vbo array
	unsigned int renderCount;
};

#endif