#include "mesh.hpp"

Vertex::Vertex(Vector3F position, Vector2F texcoord, Vector3F normal)
{
	this->position = position;
	this->texcoord = texcoord;
	this->normal = normal;
}

Mesh::Mesh(std::string filename)
{
	IndexedModel model = OBJModel(filename).ToIndexedModel();
	this->filename = filename;
	this->initMesh(model);
}

Mesh::Mesh(Vertex* vertices, unsigned int numVertices, unsigned int* indices, unsigned int numIndices)
{
	IndexedModel model;
	
	for(unsigned int i = 0; i < numVertices; i++)
	{
		model.positions.push_back(vertices[i].position);
		model.texcoords.push_back(vertices[i].texcoord);
		model.normals.push_back(vertices[i].normal);
	}
	
	for(unsigned int i = 0; i < numIndices; i++)
		model.indices.push_back(indices[i]);
	
	this->initMesh(model);
}

Mesh::~Mesh()
{
	glDeleteVertexArrays(1, &(this->vertexArrayObject));
}

std::string Mesh::getFileName()
{
	return this->filename;
}

void Mesh::render()
{
	glBindVertexArray(this->vertexArrayObject);
	glDrawElements(GL_TRIANGLES, this->renderCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

//static
std::shared_ptr<Mesh> Mesh::getFromLink(std::string meshLink, std::vector<std::shared_ptr<Mesh>> allMeshes)
{
	for(unsigned int i = 0; i < allMeshes.size(); i++)
	{
		if(allMeshes.at(i)->getFileName() == meshLink)
			return allMeshes.at(i);
	}
	return __null;
}

//private
void Mesh::initMesh(IndexedModel& model)
{
	this->renderCount = model.indices.size();
	
	glGenVertexArrays(1, &(this->vertexArrayObject));
	glBindVertexArray(this->vertexArrayObject);
	
	glGenBuffers(NUM_BUFFERS, this->vertexArrayBuffers);
	
	glBindBuffer(GL_ARRAY_BUFFER, this->vertexArrayBuffers[POSITION_VB]);
	glBufferData(GL_ARRAY_BUFFER, model.positions.size() * sizeof(model.positions[0]), &(model.positions[0]), GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, this->vertexArrayBuffers[TEXCOORD_VB]);
	glBufferData(GL_ARRAY_BUFFER, model.positions.size() * sizeof(model.texcoords[0]), &(model.texcoords[0]), GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, this->vertexArrayBuffers[NORMAL_VB]);
	glBufferData(GL_ARRAY_BUFFER, model.normals.size() * sizeof(model.normals[0]), &(model.normals[0]), GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vertexArrayBuffers[INDEX_VB]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.indices.size() * sizeof(model.indices[0]), &(model.indices[0]), GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, this->vertexArrayBuffers[TANGENT_VB]);
	glBufferData(GL_ARRAY_BUFFER, model.tangents.size() * sizeof(model.tangents[0]), &(model.tangents[0]), GL_STATIC_DRAW);

	glBindVertexArray(0);
}