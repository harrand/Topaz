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
	std::string getFileName() const;
	void render() const;
	static std::shared_ptr<Mesh> getFromLink(std::string meshLink, std::vector<std::shared_ptr<Mesh>> allMeshes);
private:
	std::string filename;
	void initMesh(IndexedModel& model);
	enum
	{
		POSITION_VB,
		TEXCOORD_VB,
		NORMAL_VB,
		INDEX_VB,
		TANGENT_VB,
		
		NUM_BUFFERS
	};
	GLuint vertexArrayObject;
	GLuint vertexArrayBuffers[NUM_BUFFERS];
	unsigned int renderCount;
};

#endif