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
	Vector3F position;
	Vector2F texcoord;
	Vector3F normal;
};

class Mesh
{
public:
	Mesh(Vertex* vertices, unsigned int numVertices, unsigned int* indices, unsigned int numIndices);
	Mesh(std::string filename = "./res/models/undefined.obj");
	~Mesh();
	IndexedModel getIndexedModel() const;
	std::string getFileName() const;
	void render() const;
	static std::shared_ptr<Mesh> getFromLink(std::string meshLink, std::vector<std::shared_ptr<Mesh>> allMeshes);
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
	GLuint vertexArrayObject;
	GLuint vertexArrayBuffers[(unsigned int)BufferTypes::NUM_BUFFERS];
	unsigned int renderCount;
};

#endif